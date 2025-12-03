#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <vision_msgs/Detection2DArray.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

ros::Publisher yolo_pub;
ros::Subscriber img_sub;

void yoloCallBack(const sensor_msgs::Image::ConstPtr& img_msg) {
  cv::Mat img = cv_bridge::toCvCopy(img_msg, "bgr8")->image;

  vision_msgs::Detection2DArray yolo_msg;

  yolo_msg.header.stamp = img_msg->header.stamp;
  yolo_msg.header.frame_id = img_msg->header.frame_id;

  vision_msgs::Detection2D det;

  det.header = yolo_msg.header;

  // det.bbox.center.x = 100;
  // det.bbox.center.y = 120;
  // det.bbox.size_x = 80;
  // det.bbox.size_y = 60;

  vision_msgs::ObjectHypothesisWithPose hyp;
  
  // hyp.id = 0;
  // hyp.score = 0.95;

  det.results.push_back(hyp);
  yolo_msg.detections.push_back(det);

  yolo_pub.publish(yolo_msg);
}

int main(int argc, char **argv)
{
  ros::init(argc, argv, "yolo_node");

  ros::NodeHandle nh;

  // Load YOLO model here

  yolo_pub = nh.advertise<vision_msgs::Detection2DArray>("/vision/detections", 1);
  img_sub = nh.subscribe("/camera/image_raw", 1, yoloCallBack);

  ros::spin();

  return 0;
}