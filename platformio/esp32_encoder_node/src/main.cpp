#include <Arduino.h>

#define ENC_A 25
#define ENC_B 26

volatile long encoderCount = 0;

void IRAM_ATTR isrA() {
  if (!digitalRead(ENC_B))
    encoderCount++;
  else
    encoderCount--;
}

void setup() {
  Serial.begin(115200);

  pinMode(ENC_A, INPUT);
  pinMode(ENC_B, INPUT);

  attachInterrupt(digitalPinToInterrupt(ENC_A), isrA, RISING);
}

void loop() {
  static long last = 0;

  if (last != encoderCount) {
    Serial.println(encoderCount);
    last = encoderCount;
  }

  delay(10);
}