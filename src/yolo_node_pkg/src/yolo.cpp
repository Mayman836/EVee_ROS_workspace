#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <vision_msgs/Detection2DArray.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <cv_bridge/cv_bridge.h>

ros::Publisher yolo_pub;
ros::Subscriber img_sub;

cv::dnn::Net yolo_net;

float confThreshold = 0.5;
float nmsThreshold = 0.4;

void yoloCallBack(const sensor_msgs::Image::ConstPtr& img_msg) {
  cv_bridge::CvImagePtr cv_ptr;
  cv_ptr = cv_bridge::toCvCopy(img_msg, "bgr8");
  cv::Mat frame = cv_ptr->image;

  cv::Mat blob = cv::dnn::blobFromImage(frame, 1/255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false);

  yolo_net.setInput(blob);
  std::vector<cv::Mat> outputs;
  yolo_net.forward(outputs, yolo_net.getUnconnectedOutLayersNames());

  vision_msgs::Detection2DArray yolo_msg;

  yolo_msg.header = img_msg->header;

  cv::Mat out = outputs[0];
  const int rows = out.size[1];

  std::vector<cv::Rect> boxes;
  std::vector<float> scores;

  for (int i = 0; i < rows; i++) {
    float conf = out.at<float>(0, i, 4);
    if (conf < confThreshold) continue;

    float cx = out.at<float>(0, i, 0) * frame.cols;
    float cy = out.at<float>(0, i, 1) * frame.rows;
    float w  = out.at<float>(0, i, 2) * frame.cols;
    float h  = out.at<float>(0, i, 3) * frame.rows;

    int left = (cx - w/2);
    int top  = (cy - h/2);

    boxes.emplace_back(left, top, w, h);
    scores.push_back(conf);
  }

  std::vector<int> indices;
  cv::dnn::NMSBoxes(boxes, scores, confThreshold, nmsThreshold, indices);



  for (int idx : indices) {
    vision_msgs::Detection2D det;
    det.header = yolo_msg.header;

    det.bbox.center.x = boxes[idx].x + boxes[idx].width/2;
    det.bbox.center.y = boxes[idx].y + boxes[idx].height/2;
    det.bbox.size_x = boxes[idx].width;
    det.bbox.size_y = boxes[idx].height;

    vision_msgs::ObjectHypothesisWithPose hyp;
    hyp.score = scores[idx];
    det.results.push_back(hyp);

    yolo_msg.detections.push_back(det);
  }

  yolo_pub.publish(yolo_msg);
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "yolo_node");

  ros::NodeHandle nh;

  yolo_net = cv::dnn::readNet("model.onnx");
  yolo_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
  yolo_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);

  yolo_pub = nh.advertise<vision_msgs::Detection2DArray>("/vision/detections", 1);
  img_sub = nh.subscribe("/camera/image_raw", 1, yoloCallBack);

  ros::spin();

  return 0;
}