#include <ros/ros.h>
#include <eigen3/Eigen/Dense>
#include <tf/transform_datatypes.h>
#include <sensor_msgs/Imu.h>
#include <sensor_msgs/NavSatFix.h>
#include <nav_msgs/Odometry.h>
#include <custom_msgs/EncoderTicks.h>
#include <random>

double wheel_radius = 0.127;
double wheelbase = 0.805;
double ticks_per_rev = 2400.0;

double lat0_deg = 30.000000;   // reference origin (can be anything)
double lon0_deg = 31.000000;

const double R_earth = 6378137.0;

// True state
double x_true = 0.0;
double y_true = 0.0;
double yaw_true = 0.0;

// Commanded motion
double v_cmd = 2.0;                 // m/s
double delta_cmd = 10.0 * M_PI/180; // 10 deg steering

std::default_random_engine gen;
std::normal_distribution<double> noise_gps(0.0, 1.5);
std::normal_distribution<double> noise_yaw(0.0, 0.01);
std::normal_distribution<double> noise_w(0.0, 0.02);
std::normal_distribution<double> noise_enc(0.0, 2.0);

int main(int argc, char **argv)
{
    ros::init(argc, argv, "fake_sensor_node");
    ros::NodeHandle nh;

    ros::Publisher pub_l = nh.advertise<custom_msgs::EncoderTicks>("/encoder/left/raw",10);
    ros::Publisher pub_r = nh.advertise<custom_msgs::EncoderTicks>("/encoder/right/raw",10);
    ros::Publisher pub_s = nh.advertise<custom_msgs::EncoderTicks>("/encoder/steering/raw",10);
    ros::Publisher pub_imu = nh.advertise<sensor_msgs::Imu>("/imu/data",10);
    ros::Publisher pub_gps = nh.advertise<sensor_msgs::NavSatFix>("/ublox/fix",10);
    ros::Publisher pub_truth = nh.advertise<nav_msgs::Odometry>("/ground_truth/odom",10);

    ros::Rate rate(50);

    ros::Time last_time = ros::Time::now();

    while(ros::ok())
    {
        ros::Time now = ros::Time::now();
        double dt = (now - last_time).toSec();
        last_time = now;

        // ---- 1. PROPAGATE TRUE STATE ----
        double yaw_dot = v_cmd / wheelbase * tan(delta_cmd);

        x_true += v_cmd * cos(yaw_true) * dt;
        y_true += v_cmd * sin(yaw_true) * dt;
        yaw_true += yaw_dot * dt;
        yaw_true = atan2(sin(yaw_true), cos(yaw_true));

        // ---- 2. ENCODERS ----
        double omega_wheel = v_cmd / wheel_radius;
        double ticks_per_sec = omega_wheel / (2*M_PI) * ticks_per_rev;
        double delta_ticks = ticks_per_sec * dt;

        custom_msgs::EncoderTicks l_msg;
        l_msg.header.stamp = now;
        l_msg.delta_ticks = delta_ticks + noise_enc(gen);

        custom_msgs::EncoderTicks r_msg = l_msg;

        custom_msgs::EncoderTicks s_msg;
        s_msg.header.stamp = now;
        s_msg.ticks = delta_cmd / (2*M_PI) * ticks_per_rev;

        pub_l.publish(l_msg);
        pub_r.publish(r_msg);
        pub_s.publish(s_msg);

        // ---- 3. IMU ----
        sensor_msgs::Imu imu_msg;
        imu_msg.header.stamp = now;
        imu_msg.header.frame_id = "base_link";

        double yaw_noisy = yaw_true + noise_yaw(gen);
        imu_msg.orientation = tf::createQuaternionMsgFromYaw(yaw_noisy);

        imu_msg.angular_velocity.z = yaw_dot + noise_w(gen);

        pub_imu.publish(imu_msg);

        // ---- 4. GPS ----
        double x_gps = x_true + noise_gps(gen);
        double y_gps = y_true + noise_gps(gen);

        double lat0 = lat0_deg * M_PI/180.0;
        double lon0 = lon0_deg * M_PI/180.0;

        double lat = lat0 + y_gps / R_earth;
        double lon = lon0 + x_gps / (R_earth * cos(lat0));

        sensor_msgs::NavSatFix gps_msg;
        gps_msg.header.stamp = now;
        gps_msg.latitude = lat * 180.0/M_PI;
        gps_msg.longitude = lon * 180.0/M_PI;
        gps_msg.status.status = sensor_msgs::NavSatStatus::STATUS_FIX;

        pub_gps.publish(gps_msg);

        // ---- 5. GROUND TRUTH ODOM ----
        nav_msgs::Odometry truth_msg;
        truth_msg.header.stamp = now;
        truth_msg.header.frame_id = "odom";
        truth_msg.child_frame_id = "base_link";

        truth_msg.pose.pose.position.x = x_true;
        truth_msg.pose.pose.position.y = y_true;
        truth_msg.pose.pose.orientation =
            tf::createQuaternionMsgFromYaw(yaw_true);

        truth_msg.twist.twist.linear.x = v_cmd;
        truth_msg.twist.twist.angular.z = yaw_dot;

        pub_truth.publish(truth_msg);

        rate.sleep();
    }

    return 0;
}