#include <Arduino.h>
#include <ros.h>
#include <geometry_msgs/Twist.h>

#include "SteeringNode.h"

ros::NodeHandle nh;

SteeringNode steering(STEP_PIN, DIR_PIN, LEFT_LIMIT_SWITCH_PIN);

// Listen for cmd_vel and apply only the angular Z component to steering
void onCmdVel(const geometry_msgs::Twist& msg) {
    steering.setTargetAngle(msg.angular.z);
}

ros::Subscriber<geometry_msgs::Twist> sub_cmd_vel("/cmd_vel", &onCmdVel);

void setup() {
    nh.getHardware()->setBaud(115200);
    nh.initNode();
    nh.subscribe(sub_cmd_vel);

    steering.init(&nh);

    // Wait for ROS connection before homing
    while (!nh.connected()) {
        nh.spinOnce();
        delay(10);
    }

    steering.home();
}

void loop() {
    // 1. Motor physics — never blocks
    steering.updateMotor();

    // 2. ROS communication
    nh.spinOnce();
    steering.publishROS();
}