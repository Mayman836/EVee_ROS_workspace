#include <ImuNode.h>

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
) {
    imu_msg.orientation.x = quat.x();
    imu_msg.orientation.y = quat.y();
    imu_msg.orientation.z = quat.z();
    imu_msg.orientation.w = quat.w();

    imu_msg.linear_acceleration.x = accel.x();
    imu_msg.linear_acceleration.y = accel.y();
    imu_msg.linear_acceleration.z = accel.z();

    imu_msg.angular_velocity.x = gyro.x();
    imu_msg.angular_velocity.y = gyro.y();
    imu_msg.angular_velocity.z = gyro.z();

    imu_msg.orientation_covariance[0] = 0.0;
    imu_msg.orientation_covariance[4] = 0.0;
    imu_msg.orientation_covariance[8] = 0.0;

    imu_msg.angular_velocity_covariance[0] = 0.01;
    imu_msg.angular_velocity_covariance[4] = 0.02;
    imu_msg.angular_velocity_covariance[8] = 0.01;

    imu_msg.linear_acceleration_covariance[0] = 0.0;
    imu_msg.linear_acceleration_covariance[4] = 0.0;
    imu_msg.linear_acceleration_covariance[8] = 0.0;

    imu_msg.header.stamp = stamp;
    imu_msg.header.frame_id = "imu_link";

    imu_pub.publish(&imu_msg);
}