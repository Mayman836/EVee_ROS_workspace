#include "MotorControlNode.h"

// --- Hardware & Physical Constants ---
const int THROTTLE_LEFT_PIN = 25;  
const int THROTTLE_RIGHT_PIN = 26; 
const float MAX_SPEED_MPS = 12.9; // Maximum safe forward speed in m/s

// --- Internal Helper Function ---
// (Not declared in the .h file because main.cpp doesn't need to see it)
int mapVelocityToDAC(float target_velocity) {
  if (target_velocity <= 0.05) {
    return 0; 
  }
  
  int dac_val = 93 + ((target_velocity / MAX_SPEED_MPS) * (255 - 93));
  return constrain(dac_val, 0, 255);
}

// --- Public Functions ---
void setupMotors() {
  // Zero the throttle on both motors at startup for safety
  dacWrite(THROTTLE_LEFT_PIN, 0);
  dacWrite(THROTTLE_RIGHT_PIN, 0);
}

void cmdVelCallback(const geometry_msgs::Twist& msg) {
  float linear_x = msg.linear.x;

  int synchronized_throttle = mapVelocityToDAC(linear_x);

  dacWrite(THROTTLE_LEFT_PIN, synchronized_throttle);
  dacWrite(THROTTLE_RIGHT_PIN, synchronized_throttle);
}