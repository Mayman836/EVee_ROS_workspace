#ifndef MOTOR_CONTROL_NODE_H
#define MOTOR_CONTROL_NODE_H

#include <Arduino.h>
#include <geometry_msgs/Twist.h>

// Initializes the DAC pins and sets motors to 0
void setupMotors();

// The callback function triggered by the ROS subscriber
void cmdVelCallback(const geometry_msgs::Twist& msg);

#endif