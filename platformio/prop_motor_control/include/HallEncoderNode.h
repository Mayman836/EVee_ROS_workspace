#ifndef HALL_ENCODER_NODE_H
#define HALL_ENCODER_NODE_H

#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include <driver/pcnt.h>

// Connect TWO Hall wires here (e.g., Yellow and Green) 60 ticks/rev
#define HALL_PIN_A 33 
#define HALL_PIN_B 32 

#define HALL_ENC           PCNT_UNIT_0
#define HALL_ENC_CHANNEL_A PCNT_CHANNEL_0
#define HALL_ENC_CHANNEL_B PCNT_CHANNEL_1

void setupMaxHall();
void readEncoder(int32_t &pulse_count);
void publishEncoder(
    int32_t pulse_count,
    ros::Publisher &pub,
    custom_msgs::EncoderTicks &msg,
    const ros::Time& stamp,
    const char* frame_id
);
void resetEncoderCount(int32_t &pulse_count);

#endif