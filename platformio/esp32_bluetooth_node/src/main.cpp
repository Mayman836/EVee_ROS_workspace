#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/Waypoint.h>
#include <BluetoothFunc.h>
#include <BluetoothConfig.h>

ros::NodeHandle nh;

custom_msgs::Waypoint wp_msg;

ros::Publisher pub("/navigation/goal", &wp_msg);

void setup() {
  Serial.begin(115200);

  setupBLE();

  nh.getHardware()->setBaud(115200);

  nh.initNode();

  nh.advertise(pub);
}

void loop() {
  if (!waypointBuffer.empty() && waypointBufferIsReady) {
    decodeWaypoints();

    waypointBuffer = "";

    waypointBufferIsReady = false;
  }

  delay(5);

  nh.spinOnce();
}