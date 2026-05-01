#include <Arduino.h>
#include <ros.h>
#include <ros/time.h>
#include <sensor_msgs/Range.h>

const int TRIG_FRONT  = 13;
const int TRIG_REAR   = 14;
const int TRIG_LEFT0  = 25;
const int TRIG_LEFT1  = 26;
const int TRIG_RIGHT0 = 27;
const int TRIG_RIGHT1 = 33;

const int ECHO_FRONT  = 34;
const int ECHO_REAR   = 35;
const int ECHO_LEFT0  = 36;
const int ECHO_LEFT1  = 39;
const int ECHO_RIGHT0 = 32; 
const int ECHO_RIGHT1 = 4;

const int NUM_SENSORS = 6;
const int trigPins[NUM_SENSORS] = {TRIG_FRONT, TRIG_REAR, TRIG_LEFT0, TRIG_LEFT1, TRIG_RIGHT0, TRIG_RIGHT1};
const int echoPins[NUM_SENSORS] = {ECHO_FRONT, ECHO_REAR, ECHO_LEFT0, ECHO_LEFT1, ECHO_RIGHT0, ECHO_RIGHT1};

const char* frame_ids[NUM_SENSORS] = {
  "ultrasonic_front_link",
  "ultrasonic_rear_link",
  "ultrasonic_left0_link",
  "ultrasonic_left1_link",
  "ultrasonic_right0_link",
  "ultrasonic_right1_link"
};

ros::NodeHandle nh;

sensor_msgs::Range range_msgs[NUM_SENSORS];

ros::Publisher pub_front("ultrasonic/front", &range_msgs[0]);
ros::Publisher pub_rear("ultrasonic/rear", &range_msgs[1]);
ros::Publisher pub_left0("ultrasonic/left0", &range_msgs[2]);
ros::Publisher pub_left1("ultrasonic/left1", &range_msgs[3]);
ros::Publisher pub_right0("ultrasonic/right0", &range_msgs[4]);
ros::Publisher pub_right1("ultrasonic/right1", &range_msgs[5]);

ros::Publisher* publishers[NUM_SENSORS] = {&pub_front, &pub_rear, &pub_left0, &pub_left1, &pub_right0, &pub_right1};

float read_distance(int trig, int echo) {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);
    digitalWrite(trig, HIGH);
    delayMicroseconds(10);
    digitalWrite(trig, LOW);

    long duration = pulseIn(echo, HIGH, 30000); 

    if (duration == 0) {
        return 4.1; 
    }

    return (duration * 0.000343) / 2.0;
}

void setup() {
    nh.getHardware()->setBaud(115200);
    nh.initNode();

    for (int i = 0; i < NUM_SENSORS; i++) {
        pinMode(trigPins[i], OUTPUT);
        pinMode(echoPins[i], INPUT);

        nh.advertise(*publishers[i]);

        range_msgs[i].header.frame_id = frame_ids[i];
        range_msgs[i].radiation_type = sensor_msgs::Range::ULTRASOUND;
        range_msgs[i].field_of_view = 0.2618; // ~15 degrees in radians
        range_msgs[i].min_range = 0.02;       // 2 cm
        range_msgs[i].max_range = 4.0;        // 4 meters
    }
}

void loop() {
    nh.spinOnce();

    ros::Time current_time = nh.now();

    for (int i = 0; i < NUM_SENSORS; i++) {
        float distance = read_distance(trigPins[i], echoPins[i]);

        range_msgs[i].header.stamp = current_time;
        range_msgs[i].range = distance;

        publishers[i]->publish(&range_msgs[i]);

        delay(10); 
    }

    delay(10);
}