#ifndef STEERING_NODE_H
#define STEERING_NODE_H

#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include <AccelStepper.h>
#include "EncoderNode.h" 

class SteeringNode {
public:
    SteeringNode(uint8_t pin_step, uint8_t pin_dir, uint8_t pin_limit);
    
    void init(ros::NodeHandle* node_handle);
    void home();
    
    // Split into two functions
    void updateMotor(); 
    void publishROS();  

    void setTargetAngle(float angle);

private:
    uint8_t step_pin;
    uint8_t dir_pin;
    uint8_t limit_pin;

    AccelStepper stepper;
    ros::NodeHandle* nh;

    custom_msgs::EncoderTicks s_enc_msg;
    ros::Publisher s_enc_pub;

    int32_t enc_count;
    float target_angle;
    
    unsigned long prev_time;
    unsigned long last_publish_time;

    static const int32_t TICK_LIMIT_RIGHT = 12000;
    static const int32_t TICK_LIMIT_LEFT = -12500;
    static const int32_t HOME_OFFSET_TICKS = 0; //12500 
    constexpr static const float MAX_RIGHT_LIMIT_DEG = 22.6;
    constexpr static const float MAX_LEFT_LIMIT_DEG = -25.7;
    static const int32_t SOFT_LIMIT_MARGIN = 25;
    constexpr static const float DEADBAND_TICKS = 5.0;   

    constexpr static const float MAX_STEP_SPEED = 1500.0;
    constexpr static const float HOMING_SPEED = -1500.0; 

    float angleToTicks(float angle);
    float ticksToAngle(int32_t ticks);
};

#endif