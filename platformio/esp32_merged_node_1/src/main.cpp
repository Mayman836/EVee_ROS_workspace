#include <Arduino.h>
#include <Wire.h>
#include <ros.h>
#include <geometry_msgs/Twist.h>
#include <std_msgs/Float32.h>
#include <sensor_msgs/Imu.h>
#include <Adafruit_BNO055.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>

#include "MotorControlNode.h"
#include "HallEncoderNode.h" 
#include "SteeringNode.h"
#include "ImuNode.h"

#define DIR_PIN 27
#define STEP_PIN 23
#define LIMIT_SWITCH_PIN 16

ros::NodeHandle nh;

// Guards every access to `nh` (spinOnce(), publish(), now()). The IMU now
// runs on its own FreeRTOS task on the second core, and rosserial's
// NodeHandle isn't safe to touch from two cores at once -- anything that
// talks to `nh` must hold this first.
SemaphoreHandle_t nh_mutex;

void onCmdVel(const geometry_msgs::Twist& msg);
ros::Subscriber<geometry_msgs::Twist> sub_cmd_vel("/cmd_vel", &onCmdVel);

custom_msgs::EncoderTicks enc_msg_1;
ros::Publisher pub_enc_1("hall_encoder_ticks_1", &enc_msg_1);

custom_msgs::EncoderTicks enc_msg_2;
ros::Publisher pub_enc_2("hall_encoder_ticks_2", &enc_msg_2);

std_msgs::Float32 vel_msg;
ros::Publisher pub_vel("motor_velocity", &vel_msg);

sensor_msgs::Imu imu_msg;
ros::Publisher imu_pub("imu/data", &imu_msg);

SteeringNode steering(STEP_PIN, DIR_PIN, LIMIT_SWITCH_PIN);
Adafruit_BNO055 bno = Adafruit_BNO055(55, 0x28);

const int TICKS_PER_REV = 60;
const float WHEEL_DIAMETER_M = 0.254; 
const float WHEEL_CIRCUMFERENCE = PI * WHEEL_DIAMETER_M;
const float METERS_PER_TICK = WHEEL_CIRCUMFERENCE / TICKS_PER_REV;

unsigned long last_time = 0;
const unsigned long PUBLISH_INTERVAL_MS = 100;
int32_t previous_ticks_1 = 0;
int32_t previous_ticks_2 = 0;

const unsigned long IMU_INTERVAL_MS = 50; // 20Hz -- now just the imuTask's own delay, not a loop() gate

// Single subscriber for /cmd_vel -- do not add a second ros::Subscriber on
// this topic. rosserial's host-side subscriber table is keyed by topic
// name, so a duplicate registration on the same name is silently ignored
// and its callback never fires. Both drive and steering are fanned out
// from here instead.
void onCmdVel(const geometry_msgs::Twist& msg) {
    setDriveThrottle(msg.linear.x);
    steering.setTargetAngle(msg.angular.z);
}

// Runs entirely on core 0, off the stepper's critical path. The slow I2C
// reads happen with the mutex NOT held, so they never block loop() on core
// 1 -- the lock is only taken for the brief moment it takes to hand the
// already-read data to rosserial.
void imuTask(void* pvParameters) {
    for (;;) {
        imu::Quaternion quat = bno.getQuat();
        imu::Vector<3> accel = bno.getVector(Adafruit_BNO055::VECTOR_LINEARACCEL);
        imu::Vector<3> gyro  = bno.getVector(Adafruit_BNO055::VECTOR_GYROSCOPE);

        if (xSemaphoreTake(nh_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            ros::Time stamp = nh.now();
            handleImu(quat, accel, gyro, imu_msg, imu_pub, stamp);
            xSemaphoreGive(nh_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(IMU_INTERVAL_MS));
    }
}

void setup() {
    setupMotors();
    setupMaxHall(); 
    
    // INTERRUPT REMOVED HERE
    
    Wire.begin();
    Wire.setClock(400000); // BNO055 supports Fast Mode I2C -- cuts time per read burst.
                            // If you see IMU read errors/dropouts, back off to 100000 or 250000.
    if (!bno.begin()) {
        while (1) delay(100);
    }
    bno.setExtCrystalUse(true);

    nh_mutex = xSemaphoreCreateMutex();

    nh.getHardware()->setBaud(115200);
    nh.initNode();
    
    nh.subscribe(sub_cmd_vel);
    
    nh.advertise(pub_enc_1);
    nh.advertise(pub_enc_2);
    nh.advertise(pub_vel);
    nh.advertise(imu_pub);

    steering.init(&nh);

    while(!nh.connected()) { 
        nh.spinOnce(); 
        delay(10);
    }
    steering.home();

    // IMU runs entirely off the main loop now -- pinned to core 0, opposite
    // the loop()/steering core (1), so its I2C reads can never stall runSpeed().
    xTaskCreatePinnedToCore(
        imuTask,
        "imuTask",
        8192,
        NULL,
        1,
        NULL,
        0
    );
}

void loop() {
    // 1. Motor Physics (runs freely at max CPU speed, zero blocking)
    steering.updateMotor();

    // 2. ROS Communication (POLLS the mutex with 0 wait time)
    // If the IMU is talking, the loop instantly skips this block and goes back to stepping.
    if (xSemaphoreTake(nh_mutex, 0) == pdTRUE) {
        nh.spinOnce();
        steering.publishROS(); 
        xSemaphoreGive(nh_mutex);
    }

    unsigned long current_time = millis();
    unsigned long dt = current_time - last_time;
    
    if (dt >= PUBLISH_INTERVAL_MS) {
        // Also pass 0 here! Never wait for the network at the expense of the motor.
        if (xSemaphoreTake(nh_mutex, 0) == pdTRUE) {
            
            int32_t current_ticks_1 = 0;
            int32_t current_ticks_2 = 0;
            readEncoders(current_ticks_1, current_ticks_2);

            if (nh.connected()) {
                int32_t delta_ticks_1 = current_ticks_1 - previous_ticks_1;
                int32_t delta_ticks_2 = current_ticks_2 - previous_ticks_2;
                
                float avg_delta_ticks = (delta_ticks_1 + delta_ticks_2) / 2.0;
                float delta_distance_m = avg_delta_ticks * METERS_PER_TICK;
                float dt_seconds = dt / 1000.0;
                float current_velocity_mps = delta_distance_m / dt_seconds;
                
                vel_msg.data = current_velocity_mps * 3.6;
                pub_vel.publish(&vel_msg);

                ros::Time stamp = nh.now();
                publishEncoder(current_ticks_1, pub_enc_1, enc_msg_1, stamp, "motor_link_1");
                publishEncoder(current_ticks_2, pub_enc_2, enc_msg_2, stamp, "motor_link_2");
            }
            xSemaphoreGive(nh_mutex);
            
            // Only update the timer if we actually successfully published!
            previous_ticks_1 = current_ticks_1;
            previous_ticks_2 = current_ticks_2;
            last_time = current_time;
        }
    }
}