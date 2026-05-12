#include <EncoderNode.h>

void setupPCNT() {
  pinMode(S_ENC_PIN_A, INPUT_PULLUP);
  pinMode(S_ENC_PIN_B, INPUT_PULLUP);

  pcnt_config_t pcnt_config = {};

  pcnt_config.pulse_gpio_num = S_ENC_PIN_A;
  pcnt_config.ctrl_gpio_num  = S_ENC_PIN_B;
  pcnt_config.unit = S_ENC;
  pcnt_config.channel = S_ENC_CHANNEL_A;
  pcnt_config.pos_mode = PCNT_COUNT_INC;
  pcnt_config.neg_mode = PCNT_COUNT_DEC;
  pcnt_config.lctrl_mode = PCNT_MODE_REVERSE;
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;
  pcnt_config.counter_h_lim = INT16_MAX;
  pcnt_config.counter_l_lim = INT16_MIN;
  pcnt_unit_config(&pcnt_config);

  pcnt_config.pulse_gpio_num = S_ENC_PIN_B;
  pcnt_config.ctrl_gpio_num  = S_ENC_PIN_A;
  pcnt_config.channel = S_ENC_CHANNEL_B;
  pcnt_config.pos_mode = PCNT_COUNT_DEC;
  pcnt_config.neg_mode = PCNT_COUNT_INC;
  pcnt_unit_config(&pcnt_config);

  pcnt_set_filter_value(S_ENC, 300);
  pcnt_filter_enable(S_ENC);

  pcnt_counter_pause(S_ENC);
  pcnt_counter_clear(S_ENC);
  pcnt_counter_resume(S_ENC);
}

void handleEncoder(
    pcnt_unit_t unit,
    int32_t &pcnt_count,
    unsigned long &last_time,
    ros::Publisher &pub,
    custom_msgs::EncoderTicks &msg,
    unsigned long now,
    const ros::Time& stamp,
    const char* frame_id
) {
    if ((now - last_time) >= PUB_INTERVAL_MS) {
        int16_t hardware_counter = 0;

        pcnt_get_counter_value(unit, &hardware_counter);
        
        pcnt_count = (int32_t)hardware_counter;

        msg.ticks = pcnt_count;
        msg.delta_ticks = 0;
        
        msg.header.stamp = stamp;
        msg.header.frame_id = frame_id;

        pub.publish(&msg);

        last_time = now;
    }
}

void resetEncoderCount(pcnt_unit_t unit, int32_t &pcnt_count) {
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);
    pcnt_counter_resume(unit);
    pcnt_count = 0;
}