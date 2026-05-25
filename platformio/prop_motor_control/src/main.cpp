#include <Arduino.h>
#include <ros.h>
#include <std_msgs/Float32.h> // Include standard float message
#include "MotorControlNode.h"
#include "HallEncoderNode.h" 

// --- ROS Setup ---
ros::NodeHandle nh;

// Motor Subscriber
ros::Subscriber<geometry_msgs::Twist> sub("/cmd_vel", &cmdVelCallback);

// Encoder Publisher
custom_msgs::EncoderTicks enc_msg;
ros::Publisher pub_enc("hall_encoder_ticks", &enc_msg);

// Velocity Publisher
std_msgs::Float32 vel_msg;
ros::Publisher pub_vel("motor_velocity", &vel_msg);

// --- Physical Constants ---
const int TICKS_PER_REV = 60;
const float WHEEL_DIAMETER_M = 0.254; // Standard 10-inch hoverboard tire..
const float WHEEL_CIRCUMFERENCE = PI * WHEEL_DIAMETER_M;
const float METERS_PER_TICK = WHEEL_CIRCUMFERENCE / TICKS_PER_REV;

// --- Timing & State Variables ---
unsigned long last_time = 0;
const unsigned long PUBLISH_INTERVAL_MS = 100; // 10Hz
int32_t previous_ticks = 0; // Remembers the tick count from the previous loop

void setup() {
  setupMotors();
  setupMaxHall(); 
  
  nh.getHardware()->setBaud(115200);
  nh.initNode();
  
  nh.subscribe(sub);
  nh.advertise(pub_enc);
  nh.advertise(pub_vel); // Register new velocity publisher

  delay(1000); 
}

void loop() {
  nh.spinOnce();

  unsigned long current_time = millis();
  unsigned long dt = current_time - last_time;
  
  if (dt >= PUBLISH_INTERVAL_MS) {
    int32_t current_ticks = 0;
    readEncoder(current_ticks);

    if (nh.connected()) {
      // 1. Calculate Ticks changed since last loop (handles forward and reverse naturally)
      int32_t delta_ticks = current_ticks - previous_ticks;
      
      // 2. Convert delta ticks to meters traveled
      float delta_distance_m = delta_ticks * METERS_PER_TICK;
      
      // 3. Calculate Velocity (v = d/t)
      float dt_seconds = dt / 1000.0;
      float current_velocity_mps = delta_distance_m / dt_seconds;
      
      // 4. Publish Velocity
      vel_msg.data = current_velocity_mps * 3.6;
      pub_vel.publish(&vel_msg);

      // 5. Publish Encoder Ticks
      ros::Time stamp = nh.now();
      publishEncoder(current_ticks, pub_enc, enc_msg, stamp, "motor_link");
    }

    // Save current state for the next calculation
    previous_ticks = current_ticks;
    last_time = current_time;
  }

  delay(10);
}