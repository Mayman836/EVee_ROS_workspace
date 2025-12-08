#include <Arduino.h>
#include <ros.h>
#include <geometry_msgs/Twist.h>

ros::NodeHandle nh;

ros::Subscriber<geometry_msgs::Twist> sub("cmd_vel", cmdVelCallback);

void cmdVelCallback(const geometry_msgs::Twist& msg) {
  float linear_x = msg.linear.x;

  float angular_z = msg.angular.z;

  // ---------------------------------------------------------
  // TODO: Write motor control code here
  // ---------------------------------------------------------
}

void setup() {
  // ---------------------------------------------------------
  // TODO: Define motor pins (PinMode)
  // ---------------------------------------------------------
  nh.getHardware()->setBaud(115200);
  
  nh.initNode();

  nh.subscribe(sub);
}

void loop() {
  nh.spinOnce();
  delay(1);
}