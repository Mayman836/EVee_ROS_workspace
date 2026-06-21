#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include "EncoderNode.h" 

ros::NodeHandle nh;

custom_msgs::EncoderTicks enc_msg;
ros::Publisher enc_pub("/encoder/steering/raw", &enc_msg);

int32_t current_ticks = 0;
unsigned long last_publish_time = 0;
const unsigned long PUBLISH_INTERVAL_MS = 50; // 20Hz publishing rate

void setup() {
    nh.getHardware()->setBaud(115200); 
    nh.initNode();
    nh.advertise(enc_pub);
    
    setupPCNT();
}

void loop() {
    unsigned long now = millis();
    
    // Non-blocking publish loop
    if (now - last_publish_time >= PUBLISH_INTERVAL_MS) {
        // 1. Read from the PCNT hardware
        readEncoder(S_ENC, current_ticks);
        
        // 2. Publish to ROS
        ros::Time stamp = nh.now();
        publishEncoder(current_ticks, enc_pub, enc_msg, stamp, "steering_wheel_link");
        
        last_publish_time = now;
    }
    
    nh.spinOnce();
}