#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include <driver/pcnt.h>
#include <string>

#ifndef ENCODER_NODE_H
#define ENCODER_NODE_H

#define L_ENC_PIN_A 25
#define L_ENC_PIN_B 26

#define L_ENC           PCNT_UNIT_0
#define L_ENC_CHANNEL_A PCNT_CHANNEL_0
#define L_ENC_CHANNEL_B PCNT_CHANNEL_1

#define R_ENC_PIN_A 32
#define R_ENC_PIN_B 33

#define R_ENC           PCNT_UNIT_1
#define R_ENC_CHANNEL_A PCNT_CHANNEL_0
#define R_ENC_CHANNEL_B PCNT_CHANNEL_1

#define S_ENC_PIN_A 27
#define S_ENC_PIN_B 17

#define S_ENC           PCNT_UNIT_2
#define S_ENC_CHANNEL_A PCNT_CHANNEL_0
#define S_ENC_CHANNEL_B PCNT_CHANNEL_1

#define PUB_INTERVAL_MS 10

void setupPCNT();

void handleEncoder(
    pcnt_unit_t unit,
    int32_t& pcnt_count,
    unsigned long& last_time,
    ros::Publisher& pub,
    custom_msgs::EncoderTicks& msg,
    unsigned long now,
    const ros::Time& stamp,
    const char* frame_id
);

#endif