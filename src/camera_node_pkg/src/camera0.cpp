#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

std::string get_pipeline() {
    return "nvarguscamerasrc sensor-id=0 ! "
           "video/x-raw(memory:NVMM), width=1280, height=720, format=NV12, framerate=30/1 ! "
           "nvvidconv flip-method=2 ! "
           "video/x-raw, width=1280, height=720, format=BGRx ! "
           "videoconvert ! "
           "video/x-raw, format=BGR ! appsink";
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "camera0_node");
    ros::NodeHandle nh;
    ros::Publisher img_pub = nh.advertise<sensor_msgs::Image>("/camera/cam0/image_raw", 1);

    cv::VideoCapture cap(get_pipeline(), cv::CAP_GSTREAMER);
    if(!cap.isOpened()) return -1;

    ros::Rate loop_rate(30);
    while(ros::ok()) {
        cv::Mat frame;
        if(cap.read(frame)) {
            sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", frame).toImageMsg();
            msg->header.stamp = ros::Time::now();
            msg->header.frame_id = "camera0_link";
            img_pub.publish(msg);
        }
        ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}