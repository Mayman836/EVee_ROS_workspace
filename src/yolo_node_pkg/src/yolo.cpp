#include <ros/ros.h>
#include <ros/package.h>
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

    ROS_INFO_THROTTLE(
        1.0,
        "YOLO preds = %d, values = %d",
        num_preds, num_vals
    );

    float conf_thresh = 0.25f;
    int detections = 0;

    for(int i = 0; i < num_preds; i++) {

        float cx = output[0 * num_preds + i];
        float cy = output[1 * num_preds + i];
        float w = output[2 * num_preds + i];
        float h = output[3 * num_preds + i];

        float best_score = 0.f;
        int best_class = -1;

        for(int c = 0; c < 3; c++) {
            float score = output[(4 + c) * num_preds + i];
            if (score > best_score) {
                best_score = score;
                best_class = c;
            }
        }

        if (best_score > conf_thresh) {
            ROS_INFO("CONF %.2f | class %d", best_score, best_class);
        }
        else {
            continue;
        }

        detections++;
    }

    ROS_INFO_THROTTLE(1.0, "Detections (raw): %d", detections);
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