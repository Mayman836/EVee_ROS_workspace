#include <utility/imumaths.h>
#include <sensor_msgs/Imu.h>
#include <ros.h>

#ifndef IMU_NODE_H
#define IMU_NODE_H

void handleImu(
    const imu::Quaternion& quat,
    const imu::Vector<3>& accel,
    const imu::Vector<3>& gyro,
    sensor_msgs::Imu& imu_msg,
    ros::Publisher& imu_pub,
    const ros::Time& stamp
    // float var_gx,
    // float var_gy,
    // float var_gz
);

#endif