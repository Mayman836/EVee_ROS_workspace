#include <Arduino.h>
// Define pin connections
const int dirPin = 27;  // DIR- connected to GPIO 2
const int stepPin = 26; // PUL- connected to GPIO 4

// Delay between pulses (determines speed). Lower = faster.
const int stepDelay = 500;

void setup() {
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(32, INPUT_PULLDOWN);

  digitalWrite(dirPin, HIGH);
  digitalWrite(stepPin, HIGH);
}

void loop() {
  while(digitalRead(32)) {
    // Set direction: Clockwise
    digitalWrite(dirPin, LOW);    
    // Send 800 pulses (Adjust based on your DM860H microstepping switch settings)
    for(int x = 0; x < 800; x++) { 
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepDelay);
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepDelay);
    }
    
    delay(1000); // 1-second pause

    // Set direction: Counter-Clockwise
    digitalWrite(dirPin, HIGH);
    
    for(int x = 0; x < 800; x++) {
      digitalWrite(stepPin, LOW);
      delayMicroseconds(stepDelay);
      digitalWrite(stepPin, HIGH);
      delayMicroseconds(stepDelay);
    }
    
    delay(1000); // 1-second pause
  }
}

// #include <Arduino.h>
// #include <ros.h>
// #include <geometry_msgs/Twist.h>

// ros::NodeHandle nh;

// ros::Subscriber<geometry_msgs::Twist> sub("cmd_vel", cmdVelCallback);

// void cmdVelCallback(const geometry_msgs::Twist& msg) {
//   float linear_x = msg.linear.x;

//   float angular_z = msg.angular.z;

//   // ---------------------------------------------------------
//   // TODO: Write motor control code here
//   // ---------------------------------------------------------
// }

// void setup() {
//   // ---------------------------------------------------------
//   // TODO: Define motor pins (PinMode)
//   // ---------------------------------------------------------
//   nh.getHardware()->setBaud(115200);
  
//   nh.initNode();

//   nh.subscribe(sub);
// }

// void loop() {
//   nh.spinOnce();

//   // ---------------------------------------------------------
//   // TODO: Loop
//   // ---------------------------------------------------------

//   delay(1);
// }