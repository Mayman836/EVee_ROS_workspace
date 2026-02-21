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

  pinMode(25, INPUT_PULLDOWN);
  pinMode(26, OUTPUT);
  pinMode(27, OUTPUT);

  setupBLE();

  nh.getHardware()->setBaud(115200);

  nh.initNode();

  nh.advertise(wp_pub);

  nh.advertise(drive_pub);
}

void loop() {
  nh.spinOnce();

  digitalWrite(26, (bleConnHandle != BLE_HS_CONN_HANDLE_NONE));
  digitalWrite(27, drive);

  if (digitalRead(25)) {
    disconnectBLE();
  }

  if (bleConnHandle != BLE_HS_CONN_HANDLE_NONE) {
    if (!waypointBuffer.empty() && waypointBufferIsReady) {
      decodeWaypoints();
      waypointBuffer = "";
      waypointBufferIsReady = false;
    }
  }

  drive_msg.data = drive;

  drive_pub.publish(&drive_msg);

  delay(5);
}