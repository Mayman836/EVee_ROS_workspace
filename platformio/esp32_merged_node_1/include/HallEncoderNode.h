#ifndef HALL_ENCODER_NODE_H
#define HALL_ENCODER_NODE_H

#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include <driver/pcnt.h>

// --- Encoder 1 ---
#define HALL1_PIN_A 13 
#define HALL1_PIN_B 14 
#define HALL1_ENC   PCNT_UNIT_1 // Moved to Unit 1 to prevent conflict!

// --- Encoder 2 ---
#define HALL2_PIN_A 18 
#define HALL2_PIN_B 19 
#define HALL2_ENC   PCNT_UNIT_2 // Moved to Unit 2

#define HALL_ENC_CHANNEL_A PCNT_CHANNEL_0
#define HALL_ENC_CHANNEL_B PCNT_CHANNEL_1

void setupMaxHall();
void readEncoders(int32_t &pulse_count1, int32_t &pulse_count2);
void resetEncoderCounts(int32_t &pulse_count1, int32_t &pulse_count2);

#endif