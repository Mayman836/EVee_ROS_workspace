#ifndef STEERING_NODE_H
#define STEERING_NODE_H

#include <Arduino.h>
#include <ros.h>
#include <std_msgs/Float64.h>
#include <custom_msgs/EncoderTicks.h>
#include <AccelStepper.h>
#include <EncoderNode.h> 

class SteeringNode {
public:
    SteeringNode(uint8_t pin_step, uint8_t pin_dir, uint8_t pin_limit);
    
    void init(ros::NodeHandle* node_handle);
    void home();
    void update();
    void setTargetAngle(float angle);
    void triggerLimit();

private:
    uint8_t step_pin;
    uint8_t dir_pin;
    uint8_t limit_pin;

    AccelStepper stepper;
    ros::NodeHandle* nh;

    // ROS Messages & Publishers
    custom_msgs::EncoderTicks s_enc_msg;
    ros::Publisher s_enc_pub;
    std_msgs::Float64 msg_target;
    ros::Publisher pub_target;
    std_msgs::Float64 msg_current;
    ros::Publisher pub_current;

    // State Variables
    int32_t enc_count;
    float target_angle;
    volatile bool limit_triggered;
    
    unsigned long prev_time;
    unsigned long last_publish_time;

    // Ackermann Calibration
    static const int32_t TICK_LIMIT_RIGHT = 6300;
    static const int32_t TICK_LIMIT_LEFT = -9600;
    static const int32_t HOME_OFFSET_TICKS = 9600; 
    constexpr static const float MAX_RIGHT_LIMIT_DEG = 14.4;
    constexpr static const float MAX_LEFT_LIMIT_DEG = -23.0;
    static const int32_t SOFT_LIMIT_MARGIN = 25;
    constexpr static const float DEADBAND_TICKS = 5.0;   

    // Motor Kinematics
    constexpr static const float MAX_STEP_SPEED = 1500.0;
    constexpr static const float HOMING_SPEED = -1500.0; 

    // Kinematic LUT Helper Functions
    float angleToTicks(float angle);
    float ticksToAngle(int32_t ticks);
};

#endif