#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <vision_msgs/Detection2DArray.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

ros::Publisher yolo_pub;
ros::Subscriber img_sub;

void yoloCallBack(const sensor_msgs::Image::ConstPtr& img_msg) {
  cv_bridge::CvImagePtr cv_ptr;
  cv_ptr = cv_bridge::toCvCopy(img_msg, "bgr8");
  cv::Mat img = cv_ptr->image;

  vision_msgs::Detection2DArray yolo_msg;

  yolo_msg.header.stamp = img_msg->header.stamp;
  yolo_msg.header.frame_id = img_msg->header.frame_id;

  vision_msgs::Detection2D det;

  det.header = yolo_msg.header;

  vision_msgs::ObjectHypothesisWithPose hyp;

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