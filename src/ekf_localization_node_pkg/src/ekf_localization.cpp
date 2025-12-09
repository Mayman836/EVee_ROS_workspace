#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/NavSatFix.h>
#include <nav_msgs/Odometry.h>

ros::Publisher ekf_pub;
ros::Subscriber imu_sub;
ros::Subscriber wheel_sub;
ros::Subscriber gps_sub;

sensor_msgs::Imu imu_msg;
nav_msgs::Odometry wheel_msg;
sensor_msgs::NavSatFix gps_msg;

void ekfPublisher() {
    nav_msgs::Odometry ekf_msg;

    ekf_msg.header.stamp = ros::Time::now();
    ekf_msg.header.frame_id = "map";
    ekf_msg.child_frame_id = "base_link";

    ekf_pub.publish(ekf_msg);
}

void imuCallBack(const sensor_msgs::Imu::ConstPtr& msg) {
    imu_msg = *msg;
    ekfPublisher();
}

void wheelCallBack(const nav_msgs::Odometry::ConstPtr& msg) {
    wheel_msg = *msg;
}

void gpsCallBack(const sensor_msgs::NavSatFix::ConstPtr& msg) {
    gps_msg = *msg;
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "ekf_localization_node");

    ros::NodeHandle nh;

    imu_sub = nh.subscribe("/imu/data", 100, imuCallBack);
    wheel_sub = nh.subscribe("/wheel/odom", 100, wheelCallBack);
    gps_sub = nh.subscribe("/gps/fix", 100, gpsCallBack);

    ekf_pub = nh.advertise<nav_msgs::Odometry>("/localization/ekf_odom", 100);

    ros::spin();

    return 0;
}