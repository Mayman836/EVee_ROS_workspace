#include "UltrasonicNode.h"

UltrasonicNode::UltrasonicNode() {
    for (int i = 0; i < NUM_SENSORS; i++) {
        sonar[i] = new NewPing(trigPins[i], echoPins[i], MAX_DISTANCE);
        publishers[i] = new ros::Publisher(topic_names[i], &range_msgs[i]);
    }
}

void UltrasonicNode::init(ros::NodeHandle& nh) {
    for (int i = 0; i < NUM_SENSORS; i++) {
        nh.advertise(*publishers[i]);

        range_msgs[i].header.frame_id = frame_ids[i];
        range_msgs[i].radiation_type = sensor_msgs::Range::ULTRASOUND;
        range_msgs[i].field_of_view = 0.26; 
        range_msgs[i].min_range = 0.02;
        range_msgs[i].max_range = (float)MAX_DISTANCE / 100.0;
    }
}

void UltrasonicNode::update(ros::NodeHandle& nh) {
    // Ping every 33ms to ensure no cross-talk and keep ROS responsive
    if (millis() - last_ping_time >= 33) {
        last_ping_time = millis();

        // Get distance in meters
        unsigned int uS = sonar[current_sensor_index]->ping();
        float distance = (uS == 0) ? (float)MAX_DISTANCE / 100.0 : (float)uS / US_ROUNDTRIP_CM / 100.0;

        range_msgs[current_sensor_index].header.stamp = nh.now();
        range_msgs[current_sensor_index].range = distance;
        
        publishers[current_sensor_index]->publish(&range_msgs[current_sensor_index]);

        // Increment to next sensor
        current_sensor_index = (current_sensor_index + 1) % NUM_SENSORS;
    }
}