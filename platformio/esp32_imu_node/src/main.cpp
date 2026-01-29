#include <Arduino.h>
#include <Wire.h>

#include <ros.h>
#include <sensor_msgs/Imu.h>

#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

ros::NodeHandle nh;
sensor_msgs::Imu imu_msg;
ros::Publisher imu_pub("imu/data", &imu_msg);

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  bno.setExtCrystalUse(true);

  nh.getHardware()->setBaud(115200);

  nh.initNode();
  nh.advertise(imu_pub);
}

void loop()
{
  if (!nh.connected()) {
    nh.spinOnce();
    delay(10);
    return;
  }

  imu::Quaternion quat = bno.getQuat();
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  imu::Vector<3> gyro  = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

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

  imu_msg.header.stamp = nh.now();
  imu_msg.header.frame_id = "imu_link";

  imu_pub.publish(&imu_msg);
  nh.spinOnce();

  delay(10);
}
