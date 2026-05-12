#ifndef ULTRASONIC_NODE_H
#define ULTRASONIC_NODE_H

#include <Arduino.h>
#include <ros.h>
#include <sensor_msgs/Range.h>
#include <NewPing.h>

class UltrasonicNode {
public:
    static const int NUM_SENSORS = 6;
    static const int MAX_DISTANCE = 400; // cm

    UltrasonicNode();
    void init(ros::NodeHandle& nh);
    void update(ros::NodeHandle& nh);

private:
    float applyCorrection(int index, float raw_cm);

    // Pins and ROS frames
    const int trigPins[NUM_SENSORS] = {13, 14, 25, 26, 27, 33};
    const int echoPins[NUM_SENSORS] = {34, 35, 36, 39, 32, 5};
    
    const char* frame_ids[NUM_SENSORS] = {
        "sonar_front", "sonar_rear", 
        "sonar_left_0", "sonar_left_1", 
        "sonar_right_0", "sonar_right_1"
    };
    
    const char* topic_names[NUM_SENSORS] = {
        "ultrasonic/front", "ultrasonic/rear",
        "ultrasonic/left0", "ultrasonic/left1",
        "ultrasonic/right0", "ultrasonic/right1"
    };

    NewPing* sonar[NUM_SENSORS];
    sensor_msgs::Range range_msgs[NUM_SENSORS];
    ros::Publisher* publishers[NUM_SENSORS];

    unsigned long last_ping_time = 0;
    int current_sensor_index = 0;
};

#endif