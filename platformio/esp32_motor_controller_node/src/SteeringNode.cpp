#include "SteeringNode.h"

SteeringNode::SteeringNode(uint8_t step_pin, uint8_t dir_pin, uint8_t limit_pin)
    : _step_pin(step_pin), _dir_pin(dir_pin), _limit_pin(limit_pin),
      _stepper(AccelStepper::DRIVER, step_pin, dir_pin),
      _s_enc_pub("/encoder/steering/raw", &_s_enc_msg),
      _pub_target("/steering/plot_target", &_msg_target),
      _pub_current("/steering/current_angle", &_msg_current) 
{
    _Kp = 1.0; _Ki = 0.0; _Kd = 0.0;
    _integral = 0.0; _prev_error = 0.0;
    _prev_time = 0; _last_publish_time = 0;
    _enc_count = 0; _enc_last_time = 0;
    _target_angle = 0.0;
    _limit_triggered = false;
}

void SteeringNode::init(ros::NodeHandle* nh) {
    _nh = nh;
    pinMode(_limit_pin, INPUT_PULLDOWN);
    
    _stepper.setPinsInverted(true, true, false); 
    _stepper.setMaxSpeed(MAX_STEP_SPEED);
    setupPCNT(); 
    
    _nh->advertise(_s_enc_pub);
    _nh->advertise(_pub_target);
    _nh->advertise(_pub_current);
}

void SteeringNode::triggerLimit() {
    _limit_triggered = true;
}

void SteeringNode::setTargetAngle(float angle) {
    _target_angle = constrain(angle, MAX_LEFT_LIMIT_DEG, MAX_RIGHT_LIMIT_DEG);
}

void SteeringNode::home() {
    _nh->loginfo("[HOMING] Phase 1: Seeking Switch (Anti-Clockwise)...");
    _limit_triggered = false; 
    _stepper.setSpeed(HOMING_SPEED);
    
    unsigned long last_spin_time = millis();

    while (true) {
        _stepper.runSpeed();
        
        if (_limit_triggered) {
            _stepper.setSpeed(0);
            _stepper.runSpeed();
            delay(5); 
            
            if (digitalRead(_limit_pin) == HIGH) {
                _nh->loginfo("[HOMING] Switch physically pressed! Moving to Phase 2.");
                break; 
            } else {
                _nh->loginfo("[HOMING] Warning: EMI noise spike detected and rejected. Resuming Phase 1.");
                _limit_triggered = false; 
                _stepper.setSpeed(HOMING_SPEED); 
            }
        }

        if (millis() - last_spin_time > 50) {
            _nh->spinOnce();
            last_spin_time = millis();
        }
        yield();
    }
    
    _stepper.setSpeed(0);
    _stepper.runSpeed();

    resetEncoderCount(S_ENC, _enc_count);
    _enc_count = 0;
    delay(200);

    _nh->loginfo("[HOMING] Phase 2: Moving to Absolute Center (Clockwise)...");
    float offset_speed = abs(HOMING_SPEED); 
    _stepper.setSpeed(offset_speed);

    unsigned long homing_enc_last_time = millis();
    last_spin_time = millis(); 
    
    while (abs(_enc_count) < HOME_OFFSET_TICKS) {
        _stepper.runSpeed();
        
        unsigned long now = millis();
        ros::Time stamp = _nh->now();
        handleEncoder(S_ENC, _enc_count, homing_enc_last_time, _s_enc_pub, _s_enc_msg, now, stamp, "steering_wheel_link");
        
        if (millis() - last_spin_time > 50) {
            _nh->spinOnce();
            last_spin_time = millis();
        }
        yield();
    }

    _stepper.setSpeed(0);
    _stepper.runSpeed();

    resetEncoderCount(S_ENC, _enc_count);
    _enc_count = 0;

    _integral = 0.0;
    _prev_error = 0.0;
    
    _enc_last_time = millis();
    _prev_time = millis();
    _nh->loginfo("[HOMING] Calibration Complete! Ready for steering commands.");
}

void SteeringNode::update() {
    unsigned long now = millis();
    ros::Time stamp = _nh->now();

    handleEncoder(S_ENC, _enc_count, _enc_last_time, _s_enc_pub, _s_enc_msg, now, stamp, "steering_wheel_link");

    float target_ticks;
    if (_target_angle < 0) {
        target_ticks = (_target_angle / MAX_LEFT_LIMIT_DEG) * (float)TICK_LIMIT_LEFT;
    } else {
        target_ticks = (_target_angle / MAX_RIGHT_LIMIT_DEG) * (float)TICK_LIMIT_RIGHT;
    }
    target_ticks = constrain(target_ticks, (float)TICK_LIMIT_LEFT + SOFT_LIMIT_MARGIN, (float)TICK_LIMIT_RIGHT - SOFT_LIMIT_MARGIN);

    float dt = (now - _prev_time) / 1000.0; 
    if (dt > 0) {
        float error = target_ticks - (float)_enc_count;
        
        if (abs(error) > DEADBAND_TICKS) {
            _integral += error * dt;
            _integral = constrain(_integral, -400.0, 400.0); 
            float derivative = (error - _prev_error) / dt;
            
            float speed = (_Kp * error) + (_Ki * _integral) + (_Kd * derivative);
            speed = constrain(speed, -MAX_STEP_SPEED, MAX_STEP_SPEED);

            _stepper.setSpeed(speed);
        } else {
            _stepper.setSpeed(0);
            _integral = 0.0; 
        }
        
        _prev_error = error;
        _prev_time = now;
    }

    _stepper.runSpeed(); 

    if (now - _last_publish_time >= 50) {
        _msg_target.data = _target_angle;
        if (_enc_count < 0) {
            _msg_current.data = ((float)_enc_count / (float)TICK_LIMIT_LEFT) * MAX_LEFT_LIMIT_DEG;
        } else {
            _msg_current.data = ((float)_enc_count / (float)TICK_LIMIT_RIGHT) * MAX_RIGHT_LIMIT_DEG;
        }
        _pub_target.publish(&_msg_target);
        _pub_current.publish(&_msg_current);
        _last_publish_time = now;
    }
}