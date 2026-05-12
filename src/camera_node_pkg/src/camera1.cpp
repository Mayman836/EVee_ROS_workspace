#include <ros/ros.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <camera_info_manager/camera_info_manager.h>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>
// expousre change
std::string get_pipeline() {
    return "nvarguscamerasrc sensor-id=1 exposuretimerange=\"100000 30000000\" gainrange=\"1 16\" ispdigitalgainrange=\"1 4\" ! "
           "video/x-raw(memory:NVMM), width=1280, height=720, format=NV12, framerate=30/1 ! "
           "nvvidconv flip-method=2 ! "
           "video/x-raw, width=1280, height=720, format=BGRx ! "
           "videoconvert ! "
           "video/x-raw, format=BGR ! appsink";
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "camera1_node");
    ros::NodeHandle nh;
    
    ros::Publisher img_pub = nh.advertise<sensor_msgs::Image>("/camera/cam1/image_raw", 1);
    ros::Publisher info_pub = nh.advertise<sensor_msgs::CameraInfo>("/camera/cam1/camera_info", 1);

    // Initialize manager and load calibration
    camera_info_manager::CameraInfoManager cinfo(nh, "cam1");
    std::string yaml_url = "file:///home/evee/ROS/calibration/left.yaml";
    if (cinfo.validateURL(yaml_url)) {
        cinfo.loadCameraInfo(yaml_url);
    } else {
        ROS_WARN("Failed to validate calibration URL for cam1");
    }

    cv::VideoCapture cap(get_pipeline(), cv::CAP_GSTREAMER);
    if(!cap.isOpened()) return -1;

    ros::Rate loop_rate(30);
    while(ros::ok()) {
        cv::Mat frame;
        if(cap.read(frame)) {
            ros::Time current_time = ros::Time::now();

            // Prepare and publish image
            sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", frame).toImageMsg();
            msg->header.stamp = current_time;
            msg->header.frame_id = "camera1_link";
            img_pub.publish(msg);

            // Prepare and publish calibration info with identical timestamp
            sensor_msgs::CameraInfo info_msg = cinfo.getCameraInfo();
            info_msg.header.stamp = current_time;
            info_msg.header.frame_id = "camera1_link";
            info_pub.publish(info_msg);
        }
        ros::spinOnce();
        loop_rate.sleep();
    }
    return 0;
}