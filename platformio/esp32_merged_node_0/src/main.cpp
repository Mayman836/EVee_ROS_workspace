#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/Waypoint.h>
#include <std_msgs/Bool.h>
#include "BluetoothNode.h"
#include "UltrasonicNode.h"

ros::NodeHandle nh;

std_msgs::Bool drive_msg;
std_msgs::Bool status_msg;
custom_msgs::Waypoint wp_msg;

ros::Publisher wp_pub("/navigation/goal", &wp_msg);
ros::Publisher drive_pub("/navigation/drive", &drive_msg);
ros::Publisher status_pub("/bluetooth/status", &status_msg);

UltrasonicNode* ultrasonic;

void setup() {
  nh.getHardware()->setBaud(115200);
  nh.initNode();

  setupBLE();

  ultrasonic = new UltrasonicNode();
  ultrasonic->init(nh);

  nh.advertise(wp_pub);
  nh.advertise(drive_pub);
  nh.advertise(status_pub);
}

void loop() {
  nh.spinOnce();

  bool isConnected = (bleConnHandle != BLE_HS_CONN_HANDLE_NONE);

  if (isConnected) {

    if (!waypointBuffer.empty() && waypointBufferIsReady) {
      decodeWaypoints();
      waypointBuffer = "";
      waypointBufferIsReady = false;
    }

    ultrasonic->update(nh);
    
  } else {
    
    drive = false; 
  }

  drive_msg.data = drive;
  drive_pub.publish(&drive_msg);

  status_msg.data = isConnected;
  status_pub.publish(&status_msg);

  delay(5);
}