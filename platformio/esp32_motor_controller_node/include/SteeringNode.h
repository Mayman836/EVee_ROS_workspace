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
    SteeringNode(uint8_t step_pin, uint8_t dir_pin, uint8_t limit_pin);
    
    void init(ros::NodeHandle* nh);
    void home();
    void update();
    void setTargetAngle(float angle);
    void triggerLimit();

private:
    uint8_t _step_pin;
    uint8_t _dir_pin;
    uint8_t _limit_pin;

    AccelStepper _stepper;
    ros::NodeHandle* _nh;

    // ROS Messages & Publishers
    custom_msgs::EncoderTicks _s_enc_msg;
    ros::Publisher _s_enc_pub;
    std_msgs::Float64 _msg_target;
    ros::Publisher _pub_target;
    std_msgs::Float64 _msg_current;
    ros::Publisher _pub_current;

    // State Variables
    int32_t _enc_count;
    unsigned long _enc_last_time;
    float _target_angle;
    volatile bool _limit_triggered;
    
    float _Kp, _Ki, _Kd;
    float _integral, _prev_error;
    unsigned long _prev_time, _last_publish_time;

    // Ackermann Calibration
    static const int32_t TICK_LIMIT_RIGHT = 6800;
    static const int32_t TICK_LIMIT_LEFT = -9700;  
    static const int32_t HOME_OFFSET_TICKS = 9600; 
    constexpr static const float MAX_RIGHT_LIMIT_DEG = 45.0;
    constexpr static const float MAX_LEFT_LIMIT_DEG = -45.0;
    static const int32_t SOFT_LIMIT_MARGIN = 50;
    constexpr static const float DEADBAND_TICKS = 10.0;   

    // Motor Kinematics
    constexpr static const float MAX_STEP_SPEED = 1500.0;
    constexpr static const float HOMING_SPEED = -1500.0; 
};

#endif