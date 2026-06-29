#ifndef STEERING_NODE_H
#define STEERING_NODE_H

#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include <AccelStepper.h>
#include "EncoderNode.h"

#define DIR_PIN                 22
#define STEP_PIN                23
#define LEFT_LIMIT_SWITCH_PIN   26
#define RIGHT_LIMIT_SWITCH_PIN  18

class SteeringNode {
public:
    SteeringNode(uint8_t pin_step, uint8_t pin_dir, uint8_t pin_limit);

    void init(ros::NodeHandle* node_handle);
    void home();

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
    float   target_angle;

    unsigned long prev_time;
    unsigned long last_publish_time;

    // ── Hard travel limits ─────────────────────────────────────────────────
    // Encoder counts LEFT  → negative, so left  limit is negative
    // Encoder counts RIGHT → positive, so right limit is positive
    static const int32_t TICK_LIMIT_LEFT            = -13100;
    static const int32_t TICK_LIMIT_RIGHT           =  11600;

    // Distance (always positive — abs is used in the homing loop) from each
    // limit switch to mechanical centre:
    //   Left  limit → centre : travel RIGHT 13100 ticks (+ve encoder direction)
    //   Right limit → centre : travel LEFT  11600 ticks (-ve encoder direction)
    static const int32_t HOME_OFFSET_TICKS_LEFT     = 13100;
    static const int32_t HOME_OFFSET_TICKS_RIGHT    = 11600;

    constexpr static const float MAX_RIGHT_LIMIT_DEG =  22.6f;
    constexpr static const float MAX_LEFT_LIMIT_DEG  = -27.1f;

    static const int32_t         SOFT_LIMIT_MARGIN  = 25;
    constexpr static const float DEADBAND_TICKS     = 5.0f;

    // ── Speed constants ────────────────────────────────────────────────────
    // Hardware reality (verified with test sketch, NO pin inversion):
    //   +ve speed → LEFT  (encoder counts negative)
    //   -ve speed → RIGHT (encoder counts positive)
    constexpr static const float MAX_STEP_SPEED = 1500.0f;
    constexpr static const float HOMING_SPEED   = 1500.0f; // +ve → LEFT → toward left limit

    // ── Right limit switch — interrupt-driven ──────────────────────────────
    // volatile: prevents compiler caching the flag in a register
    // IRAM_ATTR: ISR lives in fast SRAM — fires reliably during battery
    //            power-on transients when flash cache may be busy
    static volatile bool right_limit_hit;
    static void IRAM_ATTR rightLimitISR();

    float angleToTicks(float angle);
    float ticksToAngle(int32_t ticks);
};

#endif // STEERING_NODE_H