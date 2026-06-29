#include "SteeringNode.h"

// ─────────────────────────────────────────────────────────────────────────────
// Static member definition
// ─────────────────────────────────────────────────────────────────────────────
volatile bool SteeringNode::right_limit_hit = false;

void IRAM_ATTR SteeringNode::rightLimitISR() {
    right_limit_hit = true;
}

// ─────────────────────────────────────────────────────────────────────────────
// Calibration LUT  — 51 points, full travel range
//
// Convention:
//   LUT_TICKS  : negative = left of centre,  positive = right of centre
//   LUT_ANGLES : negative = left (deg),       positive = right (deg)
//
// Both arrays are sorted ascending so the interpolation loops work correctly.
// ─────────────────────────────────────────────────────────────────────────────
static const int NUM_DATA_POINTS = 51;

static const float LUT_TICKS[NUM_DATA_POINTS] = {
    -13100.0f, -12500.0f, -12000.0f, -11500.0f, -11000.0f,
    -10500.0f, -10000.0f,  -9500.0f,  -9000.0f,  -8500.0f,
     -8000.0f,  -7500.0f,  -7000.0f,  -6500.0f,  -6000.0f,
     -5500.0f,  -5000.0f,  -4500.0f,  -4000.0f,  -3500.0f,
     -3000.0f,  -2500.0f,  -2000.0f,  -1500.0f,  -1000.0f,
      -500.0f,     0.0f,    500.0f,   1000.0f,   1500.0f,
      2000.0f,   2500.0f,   3000.0f,   3500.0f,   4000.0f,
      4500.0f,   5000.0f,   5500.0f,   6000.0f,   6500.0f,
      7000.0f,   7500.0f,   8000.0f,   8500.0f,   9000.0f,
      9500.0f,  10000.0f,  10500.0f,  11000.0f,  11500.0f,
     11600.0f
};

static const float LUT_ANGLES[NUM_DATA_POINTS] = {
    -27.1f, -25.7f, -24.0f, -23.2f, -21.8f,
    -21.2f, -18.8f, -17.8f, -16.8f, -16.4f,
    -15.4f, -14.3f, -13.9f, -12.5f, -11.5f,
    -10.5f,  -9.5f,  -8.9f,  -7.9f,  -7.0f,
     -5.5f,  -4.4f,  -4.0f,  -3.0f,  -2.4f,
     -1.3f,   0.0f,   0.5f,   1.0f,   2.4f,
      3.0f,   4.4f,   4.8f,   6.5f,   8.0f,
      8.9f,   9.5f,  10.5f,  10.9f,  11.9f,
     12.9f,  13.7f,  14.7f,  16.4f,  16.8f,
     18.8f,  19.8f,  20.6f,  21.2f,  21.6f,
     22.6f
};

// ─────────────────────────────────────────────────────────────────────────────
// Constructor
// ─────────────────────────────────────────────────────────────────────────────
SteeringNode::SteeringNode(uint8_t pin_step, uint8_t pin_dir, uint8_t pin_limit)
    : step_pin(pin_step),
      dir_pin(pin_dir),
      limit_pin(pin_limit),
      stepper(AccelStepper::DRIVER, pin_step, pin_dir),
      s_enc_pub("/encoder/steering/raw", &s_enc_msg)
{
    prev_time         = 0;
    last_publish_time = 0;
    enc_count         = 0;
    target_angle      = 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// init()
//
// NOTE: setPinsInverted() is intentionally removed.
// Verified hardware convention with test sketch:
//   +ve speed → LEFT   (encoder goes negative)
//   -ve speed → RIGHT  (encoder goes positive)
// Inverting direction pin reversed this and caused all homing failures.
// ─────────────────────────────────────────────────────────────────────────────
void SteeringNode::init(ros::NodeHandle* node_handle) {
    nh = node_handle;

    

    // Left limit switch — polled inside the homing loop (software debounced)
    pinMode(limit_pin, INPUT_PULLDOWN);

    // Right limit switch — interrupt driven so it cannot be missed even if
    // the motor runs away on battery power-on before Phase 1 begins
    pinMode(RIGHT_LIMIT_SWITCH_PIN, INPUT_PULLDOWN);
    attachInterrupt(digitalPinToInterrupt(RIGHT_LIMIT_SWITCH_PIN),
                    rightLimitISR,
                    RISING);

    stepper.setMaxSpeed(MAX_STEP_SPEED);

    setupPCNT();
    nh->advertise(s_enc_pub);
}

// ─────────────────────────────────────────────────────────────────────────────
// home()
//
// Hardware convention recap:
//   +ve speed → LEFT  (encoder negative)
//   -ve speed → RIGHT (encoder positive)
//
// Phase 1 — seek whichever limit switch we arrive at first:
//
//   Normal   : motor goes LEFT (+1500) → hits LEFT  limit switch
//   Recovery : motor somehow goes RIGHT and hits RIGHT limit switch (ISR)
//
// Phase 2 — drive to mechanical centre from wherever we stopped:
//
//   From LEFT  limit : go RIGHT (-1500) for HOME_OFFSET_TICKS_LEFT  (13100)
//                      encoder counts +ve, abs(enc_count) climbs to 13100
//
//   From RIGHT limit : go LEFT  (+1500) for HOME_OFFSET_TICKS_RIGHT (11600)
//                      encoder counts -ve, abs(enc_count) climbs to 11600
//
// Encoder is zeroed at the limit switch and again at centre, so after homing
// enc_count == 0 always means straight-ahead.
// ─────────────────────────────────────────────────────────────────────────────
void SteeringNode::home() {

    // ── Phase 1 : seek limit switch ──────────────────────────────────────────
    nh->loginfo("[HOMING] Phase 1: Seeking left limit (going LEFT)...");

    right_limit_hit  = false;
    bool used_right_limit  = false;
    int  solid_press_count = 0;

    stepper.setSpeed(HOMING_SPEED);         // +1500 → LEFT → toward left limit
    unsigned long last_spin_time = millis();

    while (true) {
        stepper.runSpeed();

        // Right limit hit by interrupt — motor went wrong way on power-on
        if (right_limit_hit) {
            nh->logwarn("[HOMING] Right limit hit — motor went wrong direction! Recovering...");
            used_right_limit = true;
            break;
        }

        // Left limit — poll with debounce (need 100 consecutive HIGH reads)
        if (digitalRead(limit_pin) == HIGH) {
            if (++solid_press_count > 100) {
                nh->loginfo("[HOMING] Left limit found. Proceeding to Phase 2.");
                break;
            }
        } else {
            solid_press_count = 0;
        }

        if (millis() - last_spin_time > 50) {
            nh->spinOnce();
            last_spin_time = millis();
        }
        yield();
    }

    // Stop and zero encoder at whichever limit we landed on
    stepper.setSpeed(0);
    stepper.runSpeed();
    resetEncoderCount(S_ENC, enc_count);
    enc_count = 0;
    delay(200);

    // ── Phase 2 : drive to centre ─────────────────────────────────────────────
    if (used_right_limit) {
        // ── Recovery path : at RIGHT limit, need to go LEFT to reach centre ──
        // +ve speed → LEFT, encoder counts negative
        // Loop exits when abs(enc_count) reaches HOME_OFFSET_TICKS_RIGHT (11600)
        nh->loginfo("[HOMING] Phase 2 (recovery): Moving from right limit to centre (going LEFT)...");
        stepper.setSpeed(HOMING_SPEED);     // +1500 → LEFT
        last_spin_time = millis();

        while (abs(enc_count) < HOME_OFFSET_TICKS_RIGHT) {
            stepper.runSpeed();
            readEncoder(S_ENC, enc_count);

            if (millis() - last_spin_time > 50) {
                ros::Time stamp = nh->now();
                publishEncoder(enc_count, s_enc_pub, s_enc_msg, stamp, "steering_wheel_link");
                nh->spinOnce();
                last_spin_time = millis();
            }
            yield();
        }

    } else {
        // ── Normal path : at LEFT limit, need to go RIGHT to reach centre ────
        // -ve speed → RIGHT, encoder counts positive
        // Loop exits when abs(enc_count) reaches HOME_OFFSET_TICKS_LEFT (13100)
        nh->loginfo("[HOMING] Phase 2: Moving from left limit to centre (going RIGHT)...");
        stepper.setSpeed(-HOMING_SPEED);    // -1500 → RIGHT
        last_spin_time = millis();

        while (abs(enc_count) < HOME_OFFSET_TICKS_LEFT) {
            stepper.runSpeed();
            readEncoder(S_ENC, enc_count);

            if (millis() - last_spin_time > 50) {
                ros::Time stamp = nh->now();
                publishEncoder(enc_count, s_enc_pub, s_enc_msg, stamp, "steering_wheel_link");
                nh->spinOnce();
                last_spin_time = millis();
            }
            yield();
        }
    }

    // Zero encoder at true mechanical centre
    stepper.setSpeed(0);
    stepper.runSpeed();
    resetEncoderCount(S_ENC, enc_count);
    enc_count         = 0;
    prev_time         = millis();
    right_limit_hit   = false;
    nh->loginfo("[HOMING] Calibration complete! Ready for steering commands.");
}

// ─────────────────────────────────────────────────────────────────────────────
// updateMotor()
//
// Never blocks. Called every loop() iteration.
//
// Speed sign logic:
//   error = target_ticks - enc_count
//
//   Positive error → target is to the RIGHT of current position
//                  → need to go RIGHT → -ve speed
//                  → stepper.setSpeed(-error)  ← negate the error
//
//   Negative error → target is to the LEFT of current position
//                  → need to go LEFT  → +ve speed
//                  → stepper.setSpeed(-error)  ← negate gives +ve ✓
//
// So: speed = -error in all cases.
// ─────────────────────────────────────────────────────────────────────────────
void SteeringNode::updateMotor() {
    unsigned long now = millis();
    readEncoder(S_ENC, enc_count);

    float target_ticks = angleToTicks(target_angle);

    // Clamp target inside soft limits (keeps motor away from physical switches)
    target_ticks = constrain(target_ticks,
                             (float)TICK_LIMIT_LEFT  + SOFT_LIMIT_MARGIN,
                             (float)TICK_LIMIT_RIGHT - SOFT_LIMIT_MARGIN);

    float dt = (now - prev_time) / 1000.0f;
    if (dt > 0) {
        float error = target_ticks - (float)enc_count;
        if (abs(error) > DEADBAND_TICKS) {
            // Negate error: +ve error → -ve speed (RIGHT), -ve error → +ve speed (LEFT)
            stepper.setSpeed(constrain(-error, -MAX_STEP_SPEED, MAX_STEP_SPEED));
        } else {
            stepper.setSpeed(0);
        }
        prev_time = now;
    }
    stepper.runSpeed();
}

// ─────────────────────────────────────────────────────────────────────────────
// publishROS()
// Rate-limited to 20 Hz — never floods the serial link
// ─────────────────────────────────────────────────────────────────────────────
void SteeringNode::publishROS() {
    unsigned long now = millis();
    if (now - last_publish_time >= 50) {
        ros::Time stamp = nh->now();
        publishEncoder(enc_count, s_enc_pub, s_enc_msg, stamp, "steering_wheel_link");
        last_publish_time = now;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// setTargetAngle()
// ─────────────────────────────────────────────────────────────────────────────
void SteeringNode::setTargetAngle(float angle) {
    target_angle = constrain(angle, MAX_LEFT_LIMIT_DEG, MAX_RIGHT_LIMIT_DEG);
}

// ─────────────────────────────────────────────────────────────────────────────
// angleToTicks() — linear interpolation through calibration LUT
// ─────────────────────────────────────────────────────────────────────────────
float SteeringNode::angleToTicks(float angle) {
    if (angle <= LUT_ANGLES[0])                   return LUT_TICKS[0];
    if (angle >= LUT_ANGLES[NUM_DATA_POINTS - 1]) return LUT_TICKS[NUM_DATA_POINTS - 1];

    for (int i = 0; i < NUM_DATA_POINTS - 1; i++) {
        if (angle >= LUT_ANGLES[i] && angle <= LUT_ANGLES[i + 1]) {
            float t = (angle   - LUT_ANGLES[i])
                    / (LUT_ANGLES[i + 1] - LUT_ANGLES[i]);
            return LUT_TICKS[i] + t * (LUT_TICKS[i + 1] - LUT_TICKS[i]);
        }
    }
    return 0.0f;
}

// ─────────────────────────────────────────────────────────────────────────────
// ticksToAngle() — linear interpolation through calibration LUT
// ─────────────────────────────────────────────────────────────────────────────
float SteeringNode::ticksToAngle(int32_t ticks) {
    if (ticks <= (int32_t)LUT_TICKS[0])                   return LUT_ANGLES[0];
    if (ticks >= (int32_t)LUT_TICKS[NUM_DATA_POINTS - 1]) return LUT_ANGLES[NUM_DATA_POINTS - 1];

    for (int i = 0; i < NUM_DATA_POINTS - 1; i++) {
        if (ticks >= (int32_t)LUT_TICKS[i] && ticks <= (int32_t)LUT_TICKS[i + 1]) {
            float t = (float)(ticks       - (int32_t)LUT_TICKS[i])
                    / (float)((int32_t)LUT_TICKS[i + 1] - (int32_t)LUT_TICKS[i]);
            return LUT_ANGLES[i] + t * (LUT_ANGLES[i + 1] - LUT_ANGLES[i]);
        }
    }
    return 0.0f;
}