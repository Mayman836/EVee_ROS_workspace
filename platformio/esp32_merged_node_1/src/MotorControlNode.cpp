#include "MotorControlNode.h"

// GPIO25 and GPIO26 are the only two pins on this ESP32 with real onboard
// DAC hardware, so both are reserved for left/right throttle. This assumes
// STEP_PIN (main.cpp) is genuinely 23, leaving 26 free -- if you find GPIO26
// is still physically wired to the stepper, move STEP to a different free
// GPIO instead (it only needs to toggle digitally) and keep this file as is.
const int THROTTLE_PIN_LEFT = 25;
const int THROTTLE_PIN_RIGHT = 26;
const float MAX_SPEED_MPS = 12.9;

int mapVelocityToDAC(float target_velocity) {
  if (target_velocity <= 0.05) {
    return 0; 
  }
  int dac_val = 93 + ((target_velocity / MAX_SPEED_MPS) * (255 - 93));
  return constrain(dac_val, 0, 255);
}

void setupMotors() {
  dacWrite(THROTTLE_PIN_LEFT, 0);
  dacWrite(THROTTLE_PIN_RIGHT, 0);
}

void setDriveThrottle(float linear_x_mps) {
  int throttle_val = mapVelocityToDAC(linear_x_mps);
  dacWrite(THROTTLE_PIN_LEFT, throttle_val);
  dacWrite(THROTTLE_PIN_RIGHT, throttle_val);
}