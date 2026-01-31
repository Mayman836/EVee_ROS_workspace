#include <Arduino.h>
#include <ros.h>
#include <custom_msgs/EncoderTicks.h>
#include "driver/pcnt.h"

#define ENCODER_PIN_A 25
#define ENCODER_PIN_B 26

#define PCNT_UNIT      PCNT_UNIT_0
#define PCNT_CHANNEL_A PCNT_CHANNEL_0
#define PCNT_CHANNEL_B PCNT_CHANNEL_1

#define PUB_INTERVAL_MS 10

ros::NodeHandle nh;
custom_msgs::EncoderTicks encoder_msg;

ros::Publisher encoder_pub("/encoder/raw", &encoder_msg);

int32_t pcnt_count = 0;
unsigned long last_time = 0;

void setupPCNT() {
  pcnt_config_t pcnt_config = {};

  pcnt_config.pulse_gpio_num = ENCODER_PIN_A;
  pcnt_config.ctrl_gpio_num  = ENCODER_PIN_B;
  pcnt_config.unit = PCNT_UNIT;
  pcnt_config.channel = PCNT_CHANNEL_A;
  pcnt_config.pos_mode = PCNT_COUNT_INC;
  pcnt_config.neg_mode = PCNT_COUNT_DEC;
  pcnt_config.lctrl_mode = PCNT_MODE_REVERSE;
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.counter_h_lim = INT16_MAX;
  pcnt_config.counter_l_lim = INT16_MIN;
  pcnt_unit_config(&pcnt_config);

  pcnt_config.pulse_gpio_num = ENCODER_PIN_B;
  pcnt_config.ctrl_gpio_num  = ENCODER_PIN_A;
  pcnt_config.channel = PCNT_CHANNEL_B;
  pcnt_config.pos_mode = PCNT_COUNT_DEC;
  pcnt_config.neg_mode = PCNT_COUNT_INC;
  pcnt_unit_config(&pcnt_config);

  pcnt_set_filter_value(PCNT_UNIT, 100);
  pcnt_filter_enable(PCNT_UNIT);

  pcnt_counter_pause(PCNT_UNIT);
  pcnt_counter_clear(PCNT_UNIT);
  pcnt_counter_resume(PCNT_UNIT);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  nh.getHardware()->setBaud(115200);

  nh.initNode();
  nh.advertise(encoder_pub);

  setupPCNT();
  last_time = millis();
}

void loop() {
  if (!nh.connected()) {
    nh.spinOnce();
    delay(10);
    return;
  }
  
  unsigned long now = millis();

  if ((now - last_time) >= PUB_INTERVAL_MS) {
    int16_t delta = 0;

    pcnt_get_counter_value(PCNT_UNIT, &delta);
    pcnt_counter_clear(PCNT_UNIT);
    
    pcnt_count += delta;

    encoder_msg.header.stamp = nh.now();
    encoder_msg.ticks = pcnt_count;
    encoder_msg.delta_ticks = delta;

    encoder_pub.publish(&encoder_msg);

    last_time = now;
  }

  nh.spinOnce();
}