#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <onnxruntime_cxx_api.h>
#include <vector>
#include <string>

struct LetterboxInfo {
    float scale;
    int pad_x;
    int pad_y;
};

struct TrackedDet {
    cv::Rect box;
    int class_id;
    float score;
    int age;
    int missed;
};

std::vector<TrackedDet> tracked;

const float IOU_THRESH = 0.3f;
const int MAX_MISSES= 5;

LetterboxInfo letterbox(const cv::Mat& src, cv::Mat& dst, int target_size = 640) {
    int w = src.cols;
    int h = src.rows;

    float scale = std::min(target_size/(float)w, target_size/(float)h);

    int new_w = int(w * scale);
    int new_h = int(h * scale);

    cv::resize(src, dst, cv::Size(new_w, new_h));

    int pad_x = (target_size - new_w) / 2;
    int pad_y = (target_size - new_h) / 2;

    cv::copyMakeBorder(dst, dst,
        pad_y, target_size - new_h - pad_y,
        pad_x, target_size - new_w - pad_x,
        cv::BORDER_CONSTANT,
        cv::Scalar(114, 114, 114));

    return {scale, pad_x, pad_y};
}

Ort::Env ort_env(ORT_LOGGING_LEVEL_WARNING, "yolo");
Ort::Session* ort_session = nullptr;
Ort::SessionOptions ort_session_options;

bool loadModel(const std::string& model_path)
{
    try{
        ort_session_options.SetIntraOpNumThreads(1);
        ort_session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
        ort_session = new Ort::Session(ort_env, model_path.c_str(), ort_session_options);

        ROS_INFO("YOLO ONNX model loaded successfully");
        return true;
    }
    catch (const Ort::Exception& e) {
        ROS_ERROR("Failed to load ONNX model: %s", e.what());
        return false;
    }
}

float IoU(const cv::Rect& a, const cv::Rect& b) {
    int interArea = (a & b).area();
    int unionArea = a.area() + b.area() - interArea;
    return unionArea > 0 ? (float)interArea / unionArea : 0.f;
}

ros::Subscriber img_sub;

void yoloCallback(const sensor_msgs::ImageConstPtr& img_msg)
{
    cv::Mat frame = cv_bridge::toCvShare(img_msg, "bgr8")->image;
    if (frame.empty()) return;

    cv::Mat resized;
    LetterboxInfo lb = letterbox(frame, resized, 640);

    cv::cvtColor(resized, resized, cv::COLOR_BGR2RGB);

    resized.convertTo(resized, CV_32F, 1.0 / 255.0);

    std::vector<float> input_tensor(1 * 3 * 640 * 640);

    int idx = 0;
    for (int c = 0; c < 3; c++)
        for (int y = 0; y < 640; y++)
            for (int x = 0; x < 640; x++)
                input_tensor[idx++] = resized.at<cv::Vec3f>(y, x)[c];

    std::array<int64_t, 4> input_shape{1, 3, 640, 640};
    Ort::MemoryInfo mem_info =
        Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

    Ort::Value input_tensor_ort =
        Ort::Value::CreateTensor<float>(
            mem_info,
            input_tensor.data(),
            input_tensor.size(),
            input_shape.data(),
            input_shape.size()
        );

    Ort::AllocatorWithDefaultOptions allocator;

    auto input_name_alloc = ort_session->GetInputNameAllocated(0, allocator);
    auto output_name_alloc = ort_session->GetOutputNameAllocated(0, allocator);

    const char* input_name = input_name_alloc.get();
    const char* output_name = output_name_alloc.get();

    auto outputs = ort_session->Run(
        Ort::RunOptions{nullptr},
        &input_name,
        &input_tensor_ort,
        1,
        &output_name,
        1
    );

    auto info = outputs[0].GetTensorTypeAndShapeInfo();
    auto shape = info.GetShape();

    int num_vals = shape[1];
    int num_preds = shape[2];

    float* output = outputs[0].GetTensorMutableData<float>();

    float conf_thresh = 0.25f;
    float nms_thresh = 0.45;

    std::vector<cv::Rect> boxes;
    std::vector<float> scores;
    std::vector<int> class_ids;

    for(int i = 0; i < num_preds; i++) {

        float cx = output[0 * num_preds + i];
        float cy = output[1 * num_preds + i];
        float w = output[2 * num_preds + i];
        float h = output[3 * num_preds + i];

        float best_score = 0.f;
        int best_class = -1;

        int num_classes = num_vals - 4;

        for (int c = 0; c < num_classes; c++) {
            float score = output[(4 + c) * num_preds + i];
            if (score > best_score) {
                best_score = score;
                best_class = c;
            }
        }

        if (best_score < conf_thresh) continue;

        float x1 = cx - w / 2.0f;
        float y1 = cy - h / 2.0f;
        float x2 = cx + w / 2.0f;
        float y2 = cy + h / 2.0f;

        x1 = (x1 - lb.pad_x) / lb.scale;
        y1 = (y1 - lb.pad_y) / lb.scale;
        x2 = (x2 - lb.pad_x) / lb.scale;
        y2 = (y2 - lb.pad_y) / lb.scale;

        x1 = std::max(0.f, std::min(x1, (float)frame.cols));
        y1 = std::max(0.f, std::min(y1, (float)frame.rows));
        x2 = std::max(0.f, std::min(x2, (float)frame.cols));
        y2 = std::max(0.f, std::min(y2, (float)frame.rows));

        boxes.emplace_back(
            cv::Rect(
                cv::Point((int)x1, (int)y1),
                cv::Point((int)x2, (int)y2)
            )
        );

        scores.push_back(best_score);
        class_ids.push_back(best_class);
    }

    std::vector<int> keep;
    cv::dnn::NMSBoxes(
        boxes,
        scores,
        conf_thresh,
        nms_thresh,
        keep
    );

    struct Det {
        cv::Rect box;
        int class_id;
        float score;
    };

    std::vector<Det> detections;

    for (int idx : keep) {
        detections.push_back({
            boxes[idx],
            class_ids[idx],
            scores[idx]
        });
    }

    std::vector<bool> det_used(detections.size(), false);

    for (auto& t : tracked) {
        float best_iou = 0.f;
        int best_idx = -1;

        for (size_t i = 0; i < detections.size(); i++) {
            if (det_used[i]) continue;
            if (detections[i].class_id != t.class_id) continue;

            float iou = IoU(t.box, detections[i].box);
            if (iou > best_iou) {
                best_iou = iou;
                best_idx = i;
            }
        }

        if (best_iou > IOU_THRESH) {
            t.box = detections[best_idx].box;
            t.score = detections[best_idx].score;
            t.age++;
            t.missed = 0;
            det_used[best_idx] = true;
        }
        else {
            t.missed++;
        }
    }

    for (size_t i = 0; i < detections.size(); i++) {
        if (det_used[i]) continue;

        tracked.push_back({
            detections[i].box,
            detections[i].class_id,
            detections[i].score,
            1, //age
            0  //missed
        });
    }

    tracked.erase(
        std::remove_if(
            tracked.begin(),
            tracked.end(),
            [](const TrackedDet& t) {
                return t.missed > MAX_MISSES;
            }
        ),
        tracked.end()
    );

   for (size_t i = 0; i < tracked.size(); i++) {
        const auto& t = tracked[i];

        cv::rectangle(frame, t.box, cv::Scalar(0, 255, 0), 2);

        std::string label = "ID " + std::to_string(i) + " C" + std::to_string(t.class_id);

        cv::putText(
            frame,
            label,
            t.box.tl(),
            cv::FONT_HERSHEY_SIMPLEX,
            0.6,
            cv::Scalar(0, 255, 0),
            2
        );
    }

    cv::imshow("YOLO", frame);
    cv::waitKey(1);
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "yolo_node");
    ros::NodeHandle nh;

    if(!loadModel("/home/dodo/ROS/WS/evee_ws/src/yolo_node_pkg/models/best.onnx")) {
        ROS_FATAL("Could not start YOLO node");
        return -1;
    }

    img_sub = nh.subscribe("/camera/image_raw", 1, yoloCallback);

    ROS_INFO("YOLO node started (image subscriber OK)");

    ros::spin();
    return 0;
}