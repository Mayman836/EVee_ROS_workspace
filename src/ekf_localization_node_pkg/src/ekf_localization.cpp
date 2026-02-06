#include <ros/ros.h>
#include <eigen3/Eigen/Dense>
#include <tf/transform_datatypes.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/NavSatFix.h>
#include <nav_msgs/Odometry.h>
#include <custom_msgs/EncoderTicks.h>

ros::Publisher ekf_pub;
ros::Subscriber l_enc_sub;
ros::Subscriber r_enc_sub;
ros::Subscriber s_enc_sub;
ros::Subscriber imu_sub;
ros::Subscriber gps_sub;

custom_msgs::EncoderTicks l_enc_msg;
custom_msgs::EncoderTicks r_enc_msg;
custom_msgs::EncoderTicks s_enc_msg;
sensor_msgs::Imu imu_msg;
sensor_msgs::NavSatFix gps_msg;

ros::Time last_l_enc_time;
ros::Time last_r_enc_time;
ros::Time last_s_enc_time;
ros::Time last_imu_time;
ros::Time last_gps_time;

double wheel_radius;
double wheelbase;
double max_steer;

double x = 0.0;   // position x
double y = 0.0;   // position y
double yaw = 0.0; // heading
double v = 0.0;   // velocity

Eigen::MatrixXd P = Eigen::MatrixXd::Identity(4,4);

void ekfPublisher() {
    nav_msgs::Odometry ekf_msg;

    ekf_msg.header.stamp = ros::Time::now();
    ekf_msg.header.frame_id = "odom";
    ekf_msg.child_frame_id = "base_link";

    ekf_msg.pose.pose.position.x = x;
    ekf_msg.pose.pose.position.y = y;

    ekf_msg.pose.pose.orientation = tf::createQuaternionMsgFromYaw(yaw);

    ekf_msg.twist.twist.linear.x = v;

    ekf_msg.pose.covariance[0] = P(0, 0);
    ekf_msg.pose.covariance[7] = P(1, 1);
    ekf_msg.pose.covariance[35] = P(2, 2);

    ekf_pub.publish(ekf_msg);
}

void imuCallBack(const sensor_msgs::Imu::ConstPtr& msg) {
    imu_msg = *msg;
    last_imu_time = msg->header.stamp;
}

void leftEncoderCallBack(const boost::shared_ptr<const custom_msgs::EncoderTicks>& msg) {
    l_enc_msg = *msg;
    last_l_enc_time = msg->header.stamp;
}

void rightEncoderCallBack(const boost::shared_ptr<const custom_msgs::EncoderTicks>& msg) {
    r_enc_msg = *msg;
    last_r_enc_time = msg->header.stamp;
}

void steeringEncoderCallBack(const boost::shared_ptr<const custom_msgs::EncoderTicks>& msg) {
    s_enc_msg = *msg;
    last_s_enc_time = msg->header.stamp;
}

void gpsCallBack(const sensor_msgs::NavSatFix::ConstPtr& msg) {
    gps_msg = *msg;
    last_gps_time = msg->header.stamp;
}

int main(int argc, char **argv) {
    ros::init(argc, argv, "ekf_localization_node");

    ros::Time now = ros::Time::now();
    ros::Time last_l_enc_time = now;
    ros::Time last_r_enc_time = now;
    ros::Time last_s_enc_time = now;
    ros::Time last_imu_time = now;
    ros::Time last_gps_time = now;

    ros::NodeHandle nh;

    l_enc_sub = nh.subscribe("/encoder/left/raw", 10, leftEncoderCallBack);
    r_enc_sub = nh.subscribe("/encoder/right/raw", 10, rightEncoderCallBack);
    s_enc_sub = nh.subscribe("/encoder/steering/raw", 10, steeringEncoderCallBack);
    imu_sub = nh.subscribe("/imu/data", 10, imuCallBack);
    gps_sub = nh.subscribe("/ublox/fix", 10, gpsCallBack);

    ekf_pub = nh.advertise<nav_msgs::Odometry>("/localization/ekf_odom", 10);

    nh.getParam("wheel_radius", wheel_radius);
    nh.getParam("wheelbase", wheelbase);
    nh.getParam("max_steer", max_steer);

    ros::Rate loop_rate(50);

    while (ros::ok()) {
        ros::spinOnce();

        //ekfPredict();
        //ekfCorrect();
        ekfPublisher();

        loop_rate.sleep();
    }

    return 0;
}