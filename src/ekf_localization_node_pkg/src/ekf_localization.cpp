#include <ros/ros.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/NavSatFix.h>
#include <nav_msgs/Odometry.h>

ros::Publisher ekf_pub;
ros::Subscriber imu_sub;
ros::Subscriber wheel_sub;
ros::Subscriber gps_sub;

void imuCallBack(const sensor_msgs::Imu::ConstPtr& imu_msg) {
    
}

void wheelCallBack(const nav_msgs::Odometry::ConstPtr& wheel_msg) {

}

void gpsCallBack(const sensor_msgs::NavSatFix::ConstPtr& gps_msg) {

}

int main(int argc, char **argv) {
    ros::init(argc, argv, "ekf_localization_node");

    ros::NodeHandle nh;

    ekf_pub = nh.advertise<nav_msgs::Odometry>("/localization/ekf_odom", 100);
    imu_sub = nh.subscribe("/imu/data", 100, imuCallBack);
    wheel_sub = nh.subscribe("/wheel/odom", 100, wheelCallBack);
    gps_sub = nh.subscribe("/gps/fix", 100, gpsCallBack);

    ros::spin();

    return 0;
}