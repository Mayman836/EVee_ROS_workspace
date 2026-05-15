#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include "EncoderNode.h"

// --- ROS Configuration ---
ros::NodeHandle nh;
custom_msgs::EncoderTicks encoder_msg;
ros::Publisher pub_encoder("/encoder/steering/raw", &encoder_msg);

// --- State Variables ---
int32_t current_ticks = 0;
unsigned long last_pub_time = 0;
const unsigned long pub_interval = 20; // Publish at 50Hz

void setup() {
    // Initialize ROS
    nh.getHardware()->setBaud(115200);
    nh.initNode();
    nh.advertise(pub_encoder);

    // Initialize ESP32 Pulse Counter (PCNT)
    setupPCNT();

    // Wait for connection
    while (!nh.connected()) {
        nh.spinOnce();
        delay(1);
    }
}

void loop() {
    // 1. High-frequency read from hardware
    readEncoder(S_ENC, current_ticks);

    // 2. Scheduled publishing
    unsigned long now = millis();
    if (now - last_pub_time >= pub_interval) {
        last_pub_time = now;

        publishEncoder(
            current_ticks, 
            pub_encoder, 
            encoder_msg, 
            nh.now(), 
            "steering_encoder_link"
        );
    }

    // 3. Maintain ROS communication
    nh.spinOnce();
}