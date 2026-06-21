#include "SteeringNode.h"

const int NUM_DATA_POINTS = 50;

const float LUT_TICKS[NUM_DATA_POINTS] = {
-12500.0, -12000.0, -11500.0, -11000.0, -10500.0, -10000.0, -9500.0, -9000.0, -8500.0, -8000.0, -7500.0, -7000.0, -6500.0, -6000.0, -5500.0, -5000.0, -4500.0, -4000.0, -3500.0, -3000.0, -2500.0, -2000.0, -1500.0, -1000.0, -500.0, 0.0, 500.0, 1000.0, 1500.0, 2000.0, 2500.0, 3000.0, 3500.0, 4000.0, 4500.0, 5000.0, 5500.0, 6000.0, 6500.0, 7000.0, 7500.0, 8000.0, 8500.0, 9000.0, 9500.0, 10000.0, 10500.0, 11000.0, 11500.0, 12000.0
};

const float LUT_ANGLES[NUM_DATA_POINTS] = {
-25.7, -24.0, -23.2, -21.8 , -21.2 , -18.8, -17.8, -16.8, -16.4, -15.4, -14.3, -13.9, -12.5, -11.5, -10.5, -9.5, -8.9, -7.9, -7.0, -5.5, -4.4, -4.0, -3.0, -2.4, -1.3, 0.0, 0.5, 1.0, 2.4, 3.0, 4.4, 4.8, 6.5, 8.0, 8.9, 9.5, 10.5, 10.9, 11.9, 12.9, 13.7, 14.7, 16.4, 16.8, 18.8, 19.8, 20.6, 21.2, 21.6, 22.6
};


SteeringNode::SteeringNode(uint8_t pin_step, uint8_t pin_dir, uint8_t pin_limit)
    : step_pin(pin_step), dir_pin(pin_dir), limit_pin(pin_limit),
      stepper(AccelStepper::DRIVER, pin_step, pin_dir),
      s_enc_pub("/encoder/steering/raw", &s_enc_msg),
      pub_target("/steering/plot_target", &msg_target),
      pub_current("/steering/current_angle", &msg_current) 
{
    prev_time = 0; 
    last_publish_time = 0;
    enc_count = 0; 
    target_angle = 0.0;
    limit_triggered = false;
}

void SteeringNode::init(ros::NodeHandle* node_handle) {
    nh = node_handle;
    pinMode(limit_pin, INPUT_PULLDOWN);
    
    stepper.setPinsInverted(true, true, false); 
    stepper.setMaxSpeed(MAX_STEP_SPEED);
    setupPCNT(); 
    
    nh->advertise(s_enc_pub);
    nh->advertise(pub_target);
    nh->advertise(pub_current);
}

void SteeringNode::triggerLimit() {
    limit_triggered = true;
}

void SteeringNode::setTargetAngle(float angle) {
    target_angle = constrain(angle, MAX_LEFT_LIMIT_DEG, MAX_RIGHT_LIMIT_DEG);
}

// --- Linear Interpolation: Angle to Ticks ---
float SteeringNode::angleToTicks(float angle) {
    if (angle <= LUT_ANGLES[0]) return LUT_TICKS[0];
    if (angle >= LUT_ANGLES[NUM_DATA_POINTS - 1]) return LUT_TICKS[NUM_DATA_POINTS - 1];

    for (int i = 0; i < NUM_DATA_POINTS - 1; i++) {
        if (angle >= LUT_ANGLES[i] && angle <= LUT_ANGLES[i + 1]) {
            float t = (angle - LUT_ANGLES[i]) / (LUT_ANGLES[i + 1] - LUT_ANGLES[i]);
            return LUT_TICKS[i] + t * (LUT_TICKS[i + 1] - LUT_TICKS[i]);
        }
    }
    return 0.0; 
}

// --- Linear Interpolation: Ticks to Angle ---
float SteeringNode::ticksToAngle(int32_t ticks) {
    if (ticks <= LUT_TICKS[0]) return LUT_ANGLES[0];
    if (ticks >= LUT_TICKS[NUM_DATA_POINTS - 1]) return LUT_ANGLES[NUM_DATA_POINTS - 1];

    for (int i = 0; i < NUM_DATA_POINTS - 1; i++) {
        if (ticks >= LUT_TICKS[i] && ticks <= LUT_TICKS[i + 1]) {
            float t = (float)(ticks - LUT_TICKS[i]) / (float)(LUT_TICKS[i + 1] - LUT_TICKS[i]);
            return LUT_ANGLES[i] + t * (LUT_ANGLES[i + 1] - LUT_ANGLES[i]);
        }
    }
    return 0.0; 
}

void SteeringNode::home() {
    nh->loginfo("[HOMING] Phase 1: Seeking Switch (Anti-Clockwise)...");
    limit_triggered = false; 
    stepper.setSpeed(HOMING_SPEED);
    
    unsigned long last_spin_time = millis();

    while (true) {
        stepper.runSpeed();
        
        // --- BULLETPROOF EMI DEBOUNCE ---
        if (limit_triggered) {
            stepper.setSpeed(0);
            stepper.runSpeed();
            delay(5); 
            
            if (digitalRead(limit_pin) == HIGH) {
                nh->loginfo("[HOMING] Switch physically pressed! Moving to Phase 2.");
                break; 
            } else {
                nh->loginfo("[HOMING] Warning: EMI noise spike detected and rejected. Resuming Phase 1.");
                limit_triggered = false; 
                stepper.setSpeed(HOMING_SPEED); 
            }
        }

        if (millis() - last_spin_time > 50) {
            nh->spinOnce();
            last_spin_time = millis();
        }
        yield();
    }
    
    stepper.setSpeed(0);
    stepper.runSpeed();

    resetEncoderCount(S_ENC, enc_count);
    enc_count = 0;
    delay(200);

    nh->loginfo("[HOMING] Phase 2: Moving to Absolute Center (Clockwise)...");
    float offset_speed = abs(HOMING_SPEED); 
    stepper.setSpeed(offset_speed);

    last_spin_time = millis(); 
    
    while (abs(enc_count) < HOME_OFFSET_TICKS) {
        
        // --- FAST LOOP (Motor & Math) ---
        stepper.runSpeed();
        readEncoder(S_ENC, enc_count); 
        
        // --- SLOW LOOP (ROS Publishing) ---
        if (millis() - last_spin_time > 50) {
            ros::Time stamp = nh->now();
            publishEncoder(enc_count, s_enc_pub, s_enc_msg, stamp, "steering_wheel_link");
            nh->spinOnce();
            last_spin_time = millis();
        }
        yield();
    }

    stepper.setSpeed(0);
    stepper.runSpeed();

    resetEncoderCount(S_ENC, enc_count);
    enc_count = 0;

    prev_time = millis();
    
    nh->loginfo("[HOMING] Calibration Complete! Ready for steering commands.");
}

void SteeringNode::update() {
    unsigned long now = millis();
    
    readEncoder(S_ENC, enc_count);

    float target_ticks = angleToTicks(target_angle);
    target_ticks = constrain(target_ticks, (float)TICK_LIMIT_LEFT + SOFT_LIMIT_MARGIN, (float)TICK_LIMIT_RIGHT - SOFT_LIMIT_MARGIN);

    float dt = (now - prev_time) / 1000.0; 
    if (dt > 0) {
        float error = target_ticks - (float)enc_count;
        
        if (abs(error) > DEADBAND_TICKS) {
            float speed = error; // Directly mapping error to speed
            speed = constrain(speed, -MAX_STEP_SPEED, MAX_STEP_SPEED);
            stepper.setSpeed(speed);
        } else {
            stepper.setSpeed(0);
        }
        
        prev_time = now;
    }

    stepper.runSpeed(); 

    if (now - last_publish_time >= 50) {
        ros::Time stamp = nh->now();
        
        publishEncoder(enc_count, s_enc_pub, s_enc_msg, stamp, "steering_wheel_link");
        
        msg_target.data = target_angle;
        msg_current.data = ticksToAngle(enc_count); 
        
        pub_target.publish(&msg_target);
        pub_current.publish(&msg_current);
        
        last_publish_time = now;
    }
}