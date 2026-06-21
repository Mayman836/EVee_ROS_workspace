#ifndef ENCODER_NODE_H
#define ENCODER_NODE_H

#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include <driver/pcnt.h>

#define S_ENC_PIN_A 32 // White
#define S_ENC_PIN_B 33 // Green

#define S_ENC           PCNT_UNIT_0
#define S_ENC_CHANNEL_A PCNT_CHANNEL_0
#define S_ENC_CHANNEL_B PCNT_CHANNEL_1

void setupPCNT();

void readEncoder(pcnt_unit_t unit, int32_t& pcnt_count);

void publishEncoder(
    int32_t pcnt_count,
    ros::Publisher& pub,
    custom_msgs::EncoderTicks& msg,
    const ros::Time& stamp,
    const char* frame_id
);

void resetEncoderCount(pcnt_unit_t unit, int32_t &pcnt_count);

#endif