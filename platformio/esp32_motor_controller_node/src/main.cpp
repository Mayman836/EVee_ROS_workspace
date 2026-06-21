#include <Arduino.h>
#include <ros.h>
#include <std_msgs/Float64.h>
#include "SteeringNode.h"

#define DIR_PIN 27
#define STEP_PIN 23
#define LIMIT_SWITCH_PIN 16

ros::NodeHandle nh;
SteeringNode steering(STEP_PIN, DIR_PIN, LIMIT_SWITCH_PIN);

void IRAM_ATTR limitISR() {
    steering.triggerLimit();
}

// ROS Callback
void targetCb(const std_msgs::Float64& msg) {
    steering.setTargetAngle(msg.data);
}
ros::Subscriber<std_msgs::Float64> sub_target("/steering/setpoint", &targetCb);

void setup() {
    attachInterrupt(digitalPinToInterrupt(LIMIT_SWITCH_PIN), limitISR, RISING);

    nh.getHardware()->setBaud(115200);
    nh.initNode();
    nh.subscribe(sub_target);

    steering.init(&nh);

    while(!nh.connected()) { nh.spinOnce(); }

    steering.home();
}

void loop() {
    nh.spinOnce();
    steering.update();
}