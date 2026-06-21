#ifndef IMU_NODE_H
#define IMU_NODE_H

#include <Arduino.h>
#include <ros.h>
#include <sensor_msgs/Imu.h>
#include <Adafruit_BNO055.h>

void handleImu(
    const imu::Quaternion& quat,
    const imu::Vector<3>& accel,
    const imu::Vector<3>& gyro,
    sensor_msgs::Imu& imu_msg,
    ros::Publisher& imu_pub,
    const ros::Time& stamp
);

#endif