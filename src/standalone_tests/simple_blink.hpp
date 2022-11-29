#include "pins.hpp"
#pragma once

void setup() {
  Serial.begin(115200);
  Serial.println("Simple blink initiated");
  pinMode(PIN::HEALTH_CHECK_LED, OUTPUT);
}

constexpr int PERIOD = 2000;
void loop() {
  delay(PERIOD);
  Serial.println("Toggled high");
  digitalWrite(PIN::HEALTH_CHECK_LED, HIGH);
  delay(PERIOD);
  Serial.println("Toggled low");
  digitalWrite(PIN::HEALTH_CHECK_LED, LOW);
}
