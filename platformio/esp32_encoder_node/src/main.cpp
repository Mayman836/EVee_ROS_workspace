#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include <EncoderNode.h>

ros::NodeHandle nh;
// custom_msgs::EncoderTicks l_enc_msg;
// custom_msgs::EncoderTicks r_enc_msg;
custom_msgs::EncoderTicks s_enc_msg;

// ros::Publisher l_enc_pub("/encoder/left/raw", &l_enc_msg);
// ros::Publisher r_enc_pub("/encoder/right/raw", &r_enc_msg);
ros::Publisher s_enc_pub("/encoder/steering/raw", &s_enc_msg);

// int32_t l_enc_pcnt_count = 0;
// unsigned long l_enc_last_time = 0;

// int32_t r_enc_pcnt_count = 0;
// unsigned long r_enc_last_time = 0;

int32_t s_enc_pcnt_count = 0;
unsigned long s_enc_last_time = 0;

void setup() {
  Serial.begin(115200);

  nh.getHardware()->setBaud(115200);

  nh.initNode();
  // nh.advertise(l_enc_pub);
  // nh.advertise(r_enc_pub);
  nh.advertise(s_enc_pub);

  setupPCNT();

  // l_enc_last_time = millis();
  // r_enc_last_time = millis();
  s_enc_last_time = millis();
}

void loop() {
  nh.spinOnce();

  unsigned long now = millis();
  ros::Time stamp = nh.now();

  // handleEncoder(L_ENC, l_enc_pcnt_count, l_enc_last_time, l_enc_pub, l_enc_msg, now, stamp, "left_wheel_link");
  // handleEncoder(R_ENC, r_enc_pcnt_count, r_enc_last_time, r_enc_pub, r_enc_msg, now, stamp, "right_wheel_link");
  handleEncoder(S_ENC, s_enc_pcnt_count, s_enc_last_time, s_enc_pub, s_enc_msg, now, stamp, "steering_wheel_link");
  
  delay(5);
}