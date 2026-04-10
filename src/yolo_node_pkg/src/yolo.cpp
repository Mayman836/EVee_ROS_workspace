#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <NvInfer.h>
#include <cuda_runtime_api.h>
#include <fstream>

// 1. LOGGER CLASS
class Logger : public nvinfer1::ILogger {
    void log(Severity severity, const char* msg) noexcept override {
        if (severity <= Severity::kERROR) ROS_ERROR("TRT: %s", msg);
    }
} gLogger;

// 2. CONTEXT STRUCTURE
struct YoloContext {
    nvinfer1::IExecutionContext* context = nullptr;
    nvinfer1::ICudaEngine* engine = nullptr;
    nvinfer1::IRuntime* runtime = nullptr;
    void* buffers[2] = {nullptr, nullptr};
    float* host_output = nullptr;
    float* host_input = nullptr;
};

// 3. GLOBAL DECLARATION (Must be above all functions)
YoloContext* g_yolo = nullptr;
cv::Mat frame_heap, resized_heap, rgb_heap, float_heap;

// 4. INITIALIZATION FUNCTION
bool initTRT(std::string path) {
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

    // Unified Memory allocation for Orin
    cudaMallocManaged((void**)&g_yolo->host_input, 3 * 640 * 640 * sizeof(float));
    cudaMallocManaged((void**)&g_yolo->host_output, 58800 * sizeof(float));

    g_yolo->buffers[0] = g_yolo->host_input;
    g_yolo->buffers[1] = g_yolo->host_output;

    return true;
}

// 5. CALLBACK WITH MARKERS
void yoloCallback(const sensor_msgs::ImageConstPtr& msg) {
    if (!g_yolo || !g_yolo->context) return;

    try {
        ROS_INFO_ONCE("--- [Marker 1] Callback Started ---");

        cv_bridge::CvImageConstPtr cv_ptr = cv_bridge::toCvShare(msg, "bgr8");
        if (!cv_ptr || cv_ptr->image.empty()) return;

        // Use the heap-allocated Mat instead of a local one
        cv::Mat temp = cv_ptr->image;
        if (temp.isContinuous()) {
            temp.copyTo(frame_heap);
        } else {
            // This forces a brand-new, perfectly packed memory block
            frame_heap = temp.clone(); 
        }
        
        ROS_INFO_THROTTLE(2, "[Marker 2] Image Ready: %dx%d", frame_heap.cols, frame_heap.rows);

        // Explicitly check size before resizing to prevent OOB access
        if (frame_heap.cols > 0 && frame_heap.rows > 0) {
            // Force the resize to use the EXACT buffer we pre-allocated
            cv::resize(frame_heap, resized_heap, cv::Size(640, 640), 0, 0, cv::INTER_NEAREST);
            ROS_INFO_THROTTLE(2, "[Marker 2.1] Resize Done");
        }

        cv::cvtColor(resized_heap, rgb_heap, cv::COLOR_BGR2RGB);
        ROS_INFO_THROTTLE(2, "[Marker 2.2] Color Convert Done");

        rgb_heap.convertTo(float_heap, CV_32FC3, 1.0 / 255.0);
        ROS_INFO_THROTTLE(2, "[Marker 2.3] Normalization Done");
        
        ROS_INFO_THROTTLE(2, "[Marker 3] Preprocessing Done");

        // --- MEMORY SYNC TO GPU ---
        std::vector<cv::Mat> channels(3);
        for (int i = 0; i < 3; ++i) {
            channels[i] = cv::Mat(640, 640, CV_32FC1, g_yolo->host_input + (i * 640 * 640));
        }
        cv::split(float_heap, channels);
        
        ROS_INFO_THROTTLE(2, "[Marker 4] CHW Split Done");

        // --- INFERENCE ---
        g_yolo->context->executeV2(g_yolo->buffers);
        cudaDeviceSynchronize();
        
        ROS_INFO_THROTTLE(2, "[Marker 5] Inference Completed. Result[0]: %f", g_yolo->host_output[0]);

    } catch (const std::exception& e) {
        ROS_ERROR("General Exception: %s", e.what());
    }
}

// 6. MAIN
int main(int argc, char** argv) {
    cv::setUseOptimized(false);
    ros::init(argc, argv, "yolo_node");
    ros::NodeHandle nh;

    // PRE-ALLOCATE HEAP MATRICES (CRITICAL)
    frame_heap = cv::Mat(720, 1280, CV_8UC3);
    resized_heap = cv::Mat(640, 640, CV_8UC3);
    rgb_heap = cv::Mat(640, 640, CV_8UC3);
    float_heap = cv::Mat(640, 640, CV_32FC3);

    std::string path = "/home/evee/ROS/evee_ws/src/yolo_node_pkg/models/best.engine";
    if (!initTRT(path)) {
        ROS_ERROR("Could not initialize TensorRT.");
        return -1;
    }

    ros::Subscriber sub = nh.subscribe("/camera/image_raw", 1, yoloCallback);
    
    ROS_INFO("Unified Memory Node Spinning (Pre-allocated)...");
    ros::spin();
    return 0;
}