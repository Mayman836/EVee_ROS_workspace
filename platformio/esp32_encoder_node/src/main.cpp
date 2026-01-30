// PCNT

#include <Arduino.h>
#include <driver/pcnt.h>

#define ENC_A 25
#define ENC_B 26

#define PCNT_UNIT PCNT_UNIT_0

#define PUB_INTERVAL_MS 10

volatile int32_t encoderTotal = 0;
unsigned long lastPub = 0;

void setup() {
  Serial.begin(115200);

  pcnt_config_t pcnt_config = {};

  pcnt_config.pulse_gpio_num = ENC_A;
  pcnt_config.ctrl_gpio_num = ENC_B;
  pcnt_config.channel = PCNT_CHANNEL_0;
  pcnt_config.unit = PCNT_UNIT;

  pcnt_config.pos_mode = PCNT_COUNT_INC;
  pcnt_config.neg_mode = PCNT_COUNT_DEC;
  pcnt_config.lctrl_mode = PCNT_MODE_REVERSE;
  pcnt_config.hctrl_mode = PCNT_MODE_KEEP;

  pcnt_config.counter_h_lim = 32767;
  pcnt_config.counter_l_lim = -32767;

  pcnt_unit_config(&pcnt_config);

  pcnt_set_filter_value(PCNT_UNIT, 100);
  pcnt_filter_enable(PCNT_UNIT);

  pcnt_counter_pause(PCNT_UNIT);
  pcnt_counter_clear(PCNT_UNIT);
  pcnt_counter_resume(PCNT_UNIT);
}

void loop() {
  unsigned long now = millis();

  if ((now - lastPub) >= PUB_INTERVAL_MS) {
    int16_t delta = 0;

    pcnt_get_counter_value(PCNT_UNIT, &delta);
    pcnt_counter_clear(PCNT_UNIT);

    encoderTotal += delta;

    Serial.println(encoderTotal);
    lastPub = now;
  }
}

// 4x Decoding (2400 pps)

// #include <Arduino.h>

// #define ENC_A 25
// #define ENC_B 26

// volatile long encoderCount = 0;
// volatile uint8_t prevState = 0;

// const unsigned long PUB_INTERVAL_MS = 10;
// unsigned long lastPub = 0;

// const int8_t quadTable[16] = {
//   0, -1,  1,  0,
//   1,  0,  0, -1,
//  -1,  0,  0,  1,
//   0,  1, -1,  0
// };

// void IRAM_ATTR encoderISR() {
//   uint8_t state = (digitalRead(ENC_A) << 1) | digitalRead(ENC_B);
//   uint8_t index = (prevState << 2) | state;
//   encoderCount += quadTable[index];
//   prevState = state;
// }

// void setup() {
//   Serial.begin(115200);

//   pinMode(ENC_A, INPUT_PULLUP);
//   pinMode(ENC_B, INPUT_PULLUP);

//   prevState = (digitalRead(ENC_A) << 1) | digitalRead(ENC_B);

//   attachInterrupt(digitalPinToInterrupt(ENC_A), encoderISR, CHANGE);
//   attachInterrupt(digitalPinToInterrupt(ENC_B), encoderISR, CHANGE);
// }

// void loop() {
//   unsigned long now = millis();

//   if ((now - lastPub) >= PUB_INTERVAL_MS) {
//     noInterrupts();
//     long count = encoderCount;
//     interrupts();
//     Serial.println(count);
//     lastPub = now;
//   }
// }

// 1x Decoding (600 pps)

// #include <Arduino.h>

// #define ENC_A 25
// #define ENC_B 26

// volatile long encoderCount = 0;

// void IRAM_ATTR isrA() {
//   if (!digitalRead(ENC_B))
//     encoderCount++;
//   else
//     encoderCount--;
// }

// void setup() {
//   Serial.begin(115200);

//   pinMode(ENC_A, INPUT_PULLUP);
//   pinMode(ENC_B, INPUT_PULLUP);

//   attachInterrupt(digitalPinToInterrupt(ENC_A), isrA, RISING);
// }

// void loop() {
//   static long last = 0;

//   if (last != encoderCount) {
//     Serial.println(encoderCount);
//     last = encoderCount;
//   }

//   delay(10);
// }