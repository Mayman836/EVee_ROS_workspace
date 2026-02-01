#include <Arduino.h>
#include <Wire.h>
#include <ros.h>
#include <sensor_msgs/Imu.h>
#include <Adafruit_BNO055.h>
#include <ImuNode.h>

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

ros::NodeHandle nh;
sensor_msgs::Imu imu_msg;
ros::Publisher imu_pub("imu/data", &imu_msg);

void setup()
{
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin();

  if (!bno.begin()) {
    while (1) delay(100);
  }
  
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

  ros::Time stamp = nh.now();

  handleImu(quat, accel, gyro, imu_msg, imu_pub, stamp);

  nh.spinOnce();

  delay(10);
}
