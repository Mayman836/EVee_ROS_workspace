#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <vision_msgs/Detection2DArray.h>
#include <opencv2/opencv.hpp>
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include <fstream>
#include <vector>
#include <map>

// ================= CONFIGURATION =================
const int NUM_CLASSES = 3;
const int NUM_ANCHORS = 8400;
float CONF_THRESHOLD = 0.60f;     // Lowered to catch more distant objects
const float NMS_THRESHOLD = 0.40f;
const float SMOOTHING_ALPHA = 0.50f; // Slightly more responsive
const int STABILITY_THRESHOLD = 3;   // Requires 3 frames to "Confirm"
const int GRACE_PERIOD = 5;          // Can "miss" 5 frames before being deleted

const std::string CLASS_NAMES[] = {"human", "speed_bump", "vehicle"};

struct YoloContext {
    nvinfer1::IExecutionContext* context = nullptr;
    nvinfer1::ICudaEngine* engine = nullptr;
    nvinfer1::IRuntime* runtime = nullptr;
    void* buffers[2] = {nullptr, nullptr};
    float* host_input = nullptr;
    float* host_output = nullptr;
};

YoloContext* g_yolo = nullptr;
ros::Publisher pub_det;

// Tracking state
std::map<int, cv::Rect2f> g_prev_boxes;
std::map<int, int> g_detection_counters;

class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kERROR) ROS_ERROR("TRT: %s", msg);
    }
} gLogger;

bool initTRT(const std::string& path) {
    g_yolo = new YoloContext();
    std::ifstream file(path, std::ios::binary);
    if (!file.good()) return false;
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> data(size);
    file.read(data.data(), size);
    g_yolo->runtime = nvinfer1::createInferRuntime(gLogger);
    g_yolo->engine = g_yolo->runtime->deserializeCudaEngine(data.data(), size);
    if (!g_yolo->engine) return false;
    g_yolo->context = g_yolo->engine->createExecutionContext();
    cudaMallocManaged((void **)&g_yolo->host_input, 3 * 640 * 640 * sizeof(float));
    cudaMallocManaged((void **)&g_yolo->host_output, (4 + NUM_CLASSES) * NUM_ANCHORS * sizeof(float));
    g_yolo->buffers[0] = g_yolo->host_input;
    g_yolo->buffers[1] = g_yolo->host_output;
    return true;
}

void yoloCallback(const sensor_msgs::ImageConstPtr& img_msg) {
    if (!g_yolo || !g_yolo->context) return;
    
    // Drop stale frames
    double age = (ros::Time::now() - img_msg->header.stamp).toSec() * 1000.0;
    if (age > 100.0) return; 

    try {
        const uint8_t* raw_data = img_msg->data.data();
        int src_h = img_msg->height;
        int src_w = img_msg->width;
        int src_step = img_msg->step;

        // 1. Letterboxing
        float scale = std::min(640.0f / src_w, 640.0f / src_h);
        int new_w = (int)(src_w * scale);
        int new_h = (int)(src_h * scale);
        int pad_w = (640 - new_w) / 2;
        int pad_h = (640 - new_h) / 2;

        memset(g_yolo->host_input, 0, 3 * 640 * 640 * sizeof(float));
        for (int y = 0; y < new_h; ++y) {
            const uint8_t* row_ptr = raw_data + ((int)(y / scale) * src_step);
            for (int x = 0; x < new_w; ++x) {
                const uint8_t* pixel = row_ptr + ((int)(x / scale) * 3);
                int target_y = y + pad_h;
                int target_x = x + pad_w;
                g_yolo->host_input[0 * 409600 + target_y * 640 + target_x] = pixel[2] / 255.0f; 
                g_yolo->host_input[1 * 409600 + target_y * 640 + target_x] = pixel[1] / 255.0f; 
                g_yolo->host_input[2 * 409600 + target_y * 640 + target_x] = pixel[0] / 255.0f; 
            }
        }

        // 2. Inference
        g_yolo->context->executeV2(g_yolo->buffers);
        cudaDeviceSynchronize();

        // 3. Post-Process
        std::vector<int> classIds;
        std::vector<float> confidences;
        std::vector<cv::Rect> boxes;

        for (int i = 0; i < NUM_ANCHORS; ++i) {
            float max_score = 0;
            int class_id = -1;
            for (int c = 0; c < NUM_CLASSES; ++c) {
                float score = g_yolo->host_output[(4 + c) * NUM_ANCHORS + i];
                if (score > max_score) { max_score = score; class_id = c; }
            }
            if (max_score > CONF_THRESHOLD) {
                float cx = (g_yolo->host_output[0 * NUM_ANCHORS + i] - pad_w) / scale;
                float cy = (g_yolo->host_output[1 * NUM_ANCHORS + i] - pad_h) / scale;
                float w  = g_yolo->host_output[2 * NUM_ANCHORS + i] / scale;
                float h  = g_yolo->host_output[3 * NUM_ANCHORS + i] / scale;
                boxes.push_back(cv::Rect((int)(cx - w/2.0f), (int)(cy - h/2.0f), (int)w, (int)h));
                confidences.push_back(max_score);
                classIds.push_back(class_id);
            }
        }

        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, CONF_THRESHOLD, NMS_THRESHOLD, indices);

        vision_msgs::Detection2DArray yolo_msg;
        yolo_msg.header = img_msg->header;
        std::map<int, bool> found_now;

        for (int idx : indices) {
            int id = classIds[idx];
            cv::Rect2f box = boxes[idx];
            float ar = (float)box.width / box.height;

            // Aspect Ratio Guard
            if (id == 0 && ar > 1.1) continue; 
            if (id == 1 && ar < 1.0) continue; 

            // Increment counter (Limit it to Confirm + Grace to prevent overflow)
            g_detection_counters[id] = std::min(g_detection_counters[id] + 2, STABILITY_THRESHOLD + GRACE_PERIOD);
            found_now[id] = true;

            if (g_detection_counters[id] >= STABILITY_THRESHOLD) {
                // Smoothing
                if (g_prev_boxes.count(id)) {
                    box.x = (SMOOTHING_ALPHA * box.x) + (1.0f - SMOOTHING_ALPHA) * g_prev_boxes[id].x;
                    box.y = (SMOOTHING_ALPHA * box.y) + (1.0f - SMOOTHING_ALPHA) * g_prev_boxes[id].y;
                    box.width = (SMOOTHING_ALPHA * box.width) + (1.0f - SMOOTHING_ALPHA) * g_prev_boxes[id].width;
                    box.height = (SMOOTHING_ALPHA * box.height) + (1.0f - SMOOTHING_ALPHA) * g_prev_boxes[id].height;
                }
                g_prev_boxes[id] = box;

                vision_msgs::Detection2D det;
                det.header = img_msg->header;
                vision_msgs::ObjectHypothesisWithPose hyp;
                hyp.id = id;
                hyp.score = confidences[idx];
                det.results.push_back(hyp);
                det.bbox.center.x = box.x + box.width / 2.0f;
                det.bbox.center.y = box.y + box.height / 2.0f;
                det.bbox.size_x = box.width;
                det.bbox.size_y = box.height;
                yolo_msg.detections.push_back(det);

                ROS_INFO("STABLE: [%s] Conf: %.2f Pos: (%.1f, %.1f)", 
                         CLASS_NAMES[id].c_str(), hyp.score, det.bbox.center.x, det.bbox.center.y);
            } else {
                ROS_INFO_THROTTLE(0.5, "PENDING: [%s] Seen %d/%d frames", 
                                  CLASS_NAMES[id].c_str(), g_detection_counters[id], STABILITY_THRESHOLD);
            }
        }

        // Graceful Decay Logic
        for (int c = 0; c < NUM_CLASSES; ++c) {
            if (found_now.find(c) == found_now.end()) {
                g_detection_counters[c] = std::max(0, g_detection_counters[c] - 1);
            }
        }

        pub_det.publish(yolo_msg);

    } catch (...) { ROS_ERROR("YOLO Callback Exception"); }
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "yolo_node");
    ros::NodeHandle nh;
    if (!initTRT("/home/evee/ROS/evee_ws/src/yolo_node_pkg/models/best.engine")) return -1;
    pub_det = nh.advertise<vision_msgs::Detection2DArray>("/vision/detections", 1);
    ros::Subscriber sub = nh.subscribe("/camera/left/image_rect_color", 1, yoloCallback); //changed
    ROS_INFO("YOLO Node Active");
    ros::spin();
    return 0;
}