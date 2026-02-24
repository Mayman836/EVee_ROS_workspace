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

// float mean_roll = 0.0,  M2_roll = 0.0,  var_roll = 0.0;
// float mean_pitch = 0.0, M2_pitch = 0.0, var_pitch = 0.0;
// float mean_yaw = 0.0,   M2_yaw = 0.0,   var_yaw = 0.0;

// float prev_yaw = 0.0;
// bool first_yaw = true;

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

  // float qw = quat.w();
  // float qx = quat.x();
  // float qy = quat.y();
  // float qz = quat.z();

  // float roll = atan2(2.0*(qw*qx + qy*qz),
  //                   1.0 - 2.0*(qx*qx + qy*qy));

  // float sinp = 2.0*(qw*qy - qz*qx);
  // if (sinp > 1.0) sinp = 1.0;
  // if (sinp < -1.0) sinp = -1.0;
  // float pitch = asin(sinp);

  // float yaw = atan2(2.0*(qw*qz + qx*qy),
  //                   1.0 - 2.0*(qy*qy + qz*qz));


  // if (!first_yaw) {
  //   float dy = yaw - prev_yaw;
  //   if (dy > PI) yaw -= 2*PI;
  //   if (dy < -PI) yaw += 2*PI;
  // } else {
  //   first_yaw = false;
  // }
  // prev_yaw = yaw;

  // if (!covariance_ready) {

  //   sample_count++;

  //   float d_roll = roll - mean_roll;
  //   mean_roll += d_roll / sample_count;
  //   M2_roll += d_roll * (roll - mean_roll);

  //   float d_pitch = pitch - mean_pitch;
  //   mean_pitch += d_pitch / sample_count;
  //   M2_pitch += d_pitch * (pitch - mean_pitch);

  //   float d_yaw = yaw - mean_yaw;
  //   mean_yaw += d_yaw / sample_count;
  //   M2_yaw += d_yaw * (yaw - mean_yaw);

  //   if (sample_count > 3000) {
  //     var_roll  = M2_roll  / (sample_count - 1);
  //     var_pitch = M2_pitch / (sample_count - 1);
  //     var_yaw   = M2_yaw   / (sample_count - 1);

  //     covariance_ready = true;
  //   }
  // }

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
    // var_roll,
    // var_pitch,
    // var_yaw,
    // var_gx,
    // var_gy,
    // var_gz,
    );

  delay(10);
}