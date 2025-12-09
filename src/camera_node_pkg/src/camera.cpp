#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
// #include <image_transport/image_transport.h>

// image_transport::Publisher img_pub;
ros::Publisher img_pub;

int main(int argc, char **argv)
{
  ros::init(argc, argv, "camera_node");

  ros::NodeHandle nh;

  // image_transport::ImageTransport it(nh);

  // img_pub = it.advertise("/camera/image_raw", 1);

  img_pub = nh.advertise<sensor_msgs::Image>("/camera/image_raw", 1);

  cv::VideoCapture cap(0);

  if(!cap.isOpened()) {
    ROS_ERROR("Error: Could not open video device /dev/video0");
    return -1;
  }

  ROS_INFO("Camera Node Started via /dev/video0");

  ros::Rate loop_rate(30);

  while(ros::ok()) {
    cv::Mat frame;

    cap >> frame;

    if(!frame.empty()) {
      sensor_msgs::ImagePtr img_msg;

      std_msgs::Header header;

      header.stamp = ros::Time::now();
      header.frame_id = "camera_link";
      
      img_msg = cv_bridge::CvImage(header, "bgr8", frame).toImageMsg();

      img_pub.publish(img_msg);
    } else {
      ROS_WARN("Failed to capture image frame");
    }
    
    loop_rate.sleep();
  }

  cap.release();

  return 0;
}