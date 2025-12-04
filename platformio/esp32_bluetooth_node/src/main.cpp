#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/Waypoint.h>
#include <std_msgs/Bool.h>
#include <BluetoothNode.h>

ros::NodeHandle nh;

std_msgs::Bool drive_msg;

custom_msgs::Waypoint wp_msg;

ros::Publisher wp_pub("/navigation/goal", &wp_msg);

ros::Publisher drive_pub("/navigation/drive", &drive_msg);

void setup() {
  Serial.begin(115200);

  setupBLE();

  nh.getHardware()->setBaud(115200);

  nh.initNode();

  nh.advertise(wp_pub);

  nh.advertise(drive_pub);
}

void loop() {
  if (!waypointBuffer.empty() && waypointBufferIsReady) {
    decodeWaypoints();
    waypointBuffer = "";
    waypointBufferIsReady = false;
  }

  drive_msg.data = drive;

  drive_pub.publish(&drive_msg);

  delay(5);

  nh.spinOnce();
}