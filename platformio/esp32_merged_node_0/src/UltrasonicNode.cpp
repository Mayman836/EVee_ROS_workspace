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
        range_msgs[i].field_of_view = 0.26; // approx 15 degrees
        range_msgs[i].min_range = 0.02;
        range_msgs[i].max_range = (float)MAX_DISTANCE / 100.0;
    }
}

float UltrasonicNode::applyCorrection(int index, float x) {
    float corrected;
    switch(index) {
        case 0: corrected = (2e-6 * pow(x, 3)) - (0.0012 * pow(x, 2)) + (1.18 * x) - 5.0305; break;
        case 1: corrected = (7e-7 * pow(x, 3)) - (0.0002 * pow(x, 2)) + (1.0269 * x) - 0.0834; break;
        case 2: corrected = (-4e-9 * pow(x, 3)) - (8e-5 * pow(x, 2)) + (1.0355 * x) - 1.7029; break;
        case 3: corrected = (2e-6 * pow(x, 3)) - (0.0008 * pow(x, 2)) + (1.1036 * x) - 2.099; break;
        case 4: corrected = (2e-6 * pow(x, 3)) - (0.0009 * pow(x, 2)) + (1.1021 * x) - 2.2477; break;
        case 5: corrected = (1e-6 * pow(x, 3)) - (0.0004 * pow(x, 2)) + (1.0471 * x) - 1.0877; break;
        default: corrected = x; break;
    }
    // Ensure the cubic fit doesn't produce values outside physical sensor limits
    return constrain(corrected, 2.0, (float)MAX_DISTANCE);
}

void UltrasonicNode::update(ros::NodeHandle& nh) {
    // Ping every 33ms to avoid ultrasonic interference/echo overlap
    if (millis() - last_ping_time >= 33) {
        last_ping_time = millis();

        unsigned int uS = sonar[current_sensor_index]->ping();
        float final_dist_m;

        if (uS == 0) {
            // No echo received, set to max range
            final_dist_m = (float)MAX_DISTANCE / 100.0;
        } else {
            float raw_cm = (float)uS / US_ROUNDTRIP_CM;
            float corrected_cm = applyCorrection(current_sensor_index, raw_cm);
            final_dist_m = corrected_cm / 100.0;
        }

        range_msgs[current_sensor_index].header.stamp = nh.now();
        range_msgs[current_sensor_index].range = final_dist_m;
        
        publishers[current_sensor_index]->publish(&range_msgs[current_sensor_index]);

        // Cycle to the next sensor for the next update cycle
        current_sensor_index = (current_sensor_index + 1) % NUM_SENSORS;
    }
}