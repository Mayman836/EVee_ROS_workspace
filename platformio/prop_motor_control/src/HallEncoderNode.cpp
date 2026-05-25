#include <HallEncoderNode.h>

// Software accumulator to track total ticks (handles both positive and negative)
int32_t global_pulse_count = 0;

void setupMaxHall() {
    pinMode(HALL_PIN_A, INPUT_PULLUP);
    pinMode(HALL_PIN_B, INPUT_PULLUP);

    pcnt_config_t pcnt_config = {};

    // --- Configure Channel A ---
    // If A rises while B is Low -> Forward (+1)
    // If A rises while B is High -> Reverse (-1)
    pcnt_config.pulse_gpio_num = HALL_PIN_A;
    pcnt_config.ctrl_gpio_num  = HALL_PIN_B;
    pcnt_config.unit = HALL_ENC;
    pcnt_config.channel = HALL_ENC_CHANNEL_A;
    
    pcnt_config.pos_mode = PCNT_COUNT_INC;      // Base mode: Count Up on rise
    pcnt_config.neg_mode = PCNT_COUNT_DEC;      // Base mode: Count Down on fall
    pcnt_config.lctrl_mode = PCNT_MODE_KEEP;    // If B is Low, KEEP base mode
    pcnt_config.hctrl_mode = PCNT_MODE_REVERSE; // If B is High, REVERSE base mode
    
    pcnt_config.counter_h_lim = INT16_MAX;
    pcnt_config.counter_l_lim = INT16_MIN;
    pcnt_unit_config(&pcnt_config);

    // --- Configure Channel B ---
    // If B rises while A is High -> Forward (+1)
    // If B rises while A is Low -> Reverse (-1)
    pcnt_config.pulse_gpio_num = HALL_PIN_B;
    pcnt_config.ctrl_gpio_num  = HALL_PIN_A;
    pcnt_config.channel = HALL_ENC_CHANNEL_B;
    
    pcnt_config.pos_mode = PCNT_COUNT_INC;      // Base mode: Count Up on rise
    pcnt_config.neg_mode = PCNT_COUNT_DEC;      // Base mode: Count Down on fall
    pcnt_config.lctrl_mode = PCNT_MODE_REVERSE; // If A is Low, REVERSE base mode
    pcnt_config.hctrl_mode = PCNT_MODE_KEEP;    // If A is High, KEEP base mode
    
    pcnt_unit_config(&pcnt_config);

    // Hardware noise filter (Ignores electrical bounce)
    pcnt_set_filter_value(HALL_ENC, 1000);
    pcnt_filter_enable(HALL_ENC);

    pcnt_counter_pause(HALL_ENC);
    pcnt_counter_clear(HALL_ENC);
    pcnt_counter_resume(HALL_ENC);
}

void readEncoder(int32_t &pulse_count) {
    int16_t hardware_counter = 0;
    
    // 1. Read the hardware counter (can be positive or negative)
    pcnt_get_counter_value(HALL_ENC, &hardware_counter);
    
    // 2. Clear hardware immediately to prevent overflow
    pcnt_counter_clear(HALL_ENC);
    
    // 3. Accumulate ticks. If spinning backward, hardware_counter is negative, 
    //    so adding it mathematically subtracts from the global count.
    global_pulse_count += hardware_counter;
    
    // 4. Output the value
    pulse_count = global_pulse_count;
}

void publishEncoder(
    int32_t pulse_count,
    ros::Publisher &pub,
    custom_msgs::EncoderTicks &msg,
    const ros::Time& stamp,
    const char* frame_id
) {
    msg.ticks = pulse_count;
    msg.delta_ticks = 0; 
    
    msg.header.stamp = stamp;
    msg.header.frame_id = frame_id;

    pub.publish(&msg);
}

void resetEncoderCount(int32_t &pulse_count) {
    pcnt_counter_pause(HALL_ENC);
    pcnt_counter_clear(HALL_ENC);
    pcnt_counter_resume(HALL_ENC);
    global_pulse_count = 0;
    pulse_count = 0;
}