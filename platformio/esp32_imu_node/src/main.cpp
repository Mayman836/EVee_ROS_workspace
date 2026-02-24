#include <Arduino.h>
#include <Wire.h>
#include <ros.h>
#include <sensor_msgs/Imu.h>
#include <Adafruit_BNO055.h>
#include <ImuNode.h>

// SDA -> 21
// SCL -> 22

// int sample_count = 0;
// bool covariance_ready = false;

// float mean_gx = 0.0;
// float M2_gx = 0.0;
// float var_gx = 0.0;

// float mean_gy = 0.0;
// float M2_gy = 0.0;
// float var_gy = 0.0;

// float mean_gz = 0.0;
// float M2_gz = 0.0;
// float var_gz = 0.0;

Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

ros::NodeHandle nh;
sensor_msgs::Imu imu_msg;
ros::Publisher imu_pub("imu/data", &imu_msg);

void setup()
{
  Serial.begin(115200);

  Wire.begin();

  if (!bno.begin()) {
    while (1) delay(100);
  }
  
  bno.setExtCrystalUse(true);

  nh.getHardware()->setBaud(115200);

  nh.initNode();
  nh.advertise(imu_pub);

  // delay(5000);
}

void loop()
{
  nh.spinOnce();
  
  imu::Quaternion quat = bno.getQuat();
  imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
  imu::Vector<3> gyro  = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

  ros::Time stamp = nh.now();

  // if (!covariance_ready) {
  //   sample_count++;

  //   float gx = gyro.x();

  //   float delta_x = gx - mean_gx;
  //   mean_gx += delta_x / sample_count;
  //   M2_gx += delta_x * (gx - mean_gx);

  //   float gy = gyro.y();

  //   float delta_y = gy - mean_gy;
  //   mean_gy += delta_y / sample_count;
  //   M2_gy += delta_y * (gy - mean_gy);

  //   float gz = gyro.z();

  //   float delta_z = gz - mean_gz;
  //   mean_gz += delta_z / sample_count;
  //   M2_gz += delta_z * (gz - mean_gz);

  //   if (sample_count > 1000) {
  //     var_gx = M2_gx / (sample_count - 1);
  //     var_gy = M2_gy / (sample_count - 1);
  //     var_gz = M2_gz / (sample_count - 1);
  //     covariance_ready = true;
  //   }
  // }

  handleImu(
    quat,
    accel,
    gyro,
    imu_msg,
    imu_pub,
    stamp
    // var_gx,
    // var_gy,
    // var_gz
    );

  delay(10);
}