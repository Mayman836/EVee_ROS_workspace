#include "HallEncoderNode.h"
#include "EncoderNode.h" 

int32_t global_pulse_count_1 = 0;
int32_t global_pulse_count_2 = 0;

void configurePCNT(pcnt_unit_t unit, int pinA, int pinB) {
    pcnt_config_t pcnt_config = {};

    pcnt_config.pulse_gpio_num = pinA;
    pcnt_config.ctrl_gpio_num  = pinB;
    pcnt_config.unit = unit;
    pcnt_config.channel = HALL_ENC_CHANNEL_A;
    pcnt_config.pos_mode = PCNT_COUNT_INC;      
    pcnt_config.neg_mode = PCNT_COUNT_DEC;      
    pcnt_config.lctrl_mode = PCNT_MODE_KEEP;    
    pcnt_config.hctrl_mode = PCNT_MODE_REVERSE; 
    pcnt_config.counter_h_lim = INT16_MAX;
    pcnt_config.counter_l_lim = INT16_MIN;
    pcnt_unit_config(&pcnt_config);

    pcnt_config.pulse_gpio_num = pinB;
    pcnt_config.ctrl_gpio_num  = pinA;
    pcnt_config.channel = HALL_ENC_CHANNEL_B;
    pcnt_config.pos_mode = PCNT_COUNT_INC;      
    pcnt_config.neg_mode = PCNT_COUNT_DEC;      
    pcnt_config.lctrl_mode = PCNT_MODE_REVERSE; 
    pcnt_config.hctrl_mode = PCNT_MODE_KEEP;    
    pcnt_unit_config(&pcnt_config);

    pcnt_set_filter_value(unit, 1000);
    pcnt_filter_enable(unit);
    pcnt_counter_pause(unit);
    pcnt_counter_clear(unit);
    pcnt_counter_resume(unit);
}

void setupMaxHall() {
    pinMode(HALL1_PIN_A, INPUT_PULLUP);
    pinMode(HALL1_PIN_B, INPUT_PULLUP);
    pinMode(HALL2_PIN_A, INPUT_PULLUP);
    pinMode(HALL2_PIN_B, INPUT_PULLUP);

    configurePCNT(HALL1_ENC, HALL1_PIN_A, HALL1_PIN_B);
    configurePCNT(HALL2_ENC, HALL2_PIN_A, HALL2_PIN_B);
}

void readEncoders(int32_t &pulse_count1, int32_t &pulse_count2) {
    int16_t hardware_counter_1 = 0;
    int16_t hardware_counter_2 = 0;
    
    pcnt_get_counter_value(HALL1_ENC, &hardware_counter_1);
    pcnt_counter_clear(HALL1_ENC);
    global_pulse_count_1 += hardware_counter_1;
    pulse_count1 = global_pulse_count_1;

    pcnt_get_counter_value(HALL2_ENC, &hardware_counter_2);
    pcnt_counter_clear(HALL2_ENC);
    global_pulse_count_2 += hardware_counter_2;
    pulse_count2 = global_pulse_count_2;
}

void resetEncoderCounts(int32_t &pulse_count1, int32_t &pulse_count2) {
    pcnt_counter_pause(HALL1_ENC);
    pcnt_counter_clear(HALL1_ENC);
    pcnt_counter_resume(HALL1_ENC);
    
    pcnt_counter_pause(HALL2_ENC);
    pcnt_counter_clear(HALL2_ENC);
    pcnt_counter_resume(HALL2_ENC);
    
    global_pulse_count_1 = 0;
    global_pulse_count_2 = 0;
    pulse_count1 = 0;
    pulse_count2 = 0;
}