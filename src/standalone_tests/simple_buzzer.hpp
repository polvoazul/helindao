#include "pins.hpp"
#pragma once


void a_setup() {
 Musician musician = Musician( PIN::HEALTH_CHECK_LED, LEDC_CHANNEL_BUZZER);
 const char melody_simple[] = "(cge+)*";
 Melody melody;
  delay(200);
  Serial.begin(115200);
  Serial.println("Simple buzzer initiated");
  pinMode(PIN::HEALTH_CHECK_LED, OUTPUT);
  melody.setScore(melody_simple);
  melody.setTempo(200);
  musician.setMelody(&melody);
  musician.setBreath(90);
  musician.play();
  musician.refresh();
}

void a_loop() {
 Musician musician = Musician( PIN::HEALTH_CHECK_LED, LEDC_CHANNEL_BUZZER);
  musician.refresh();
  static int last = 0;
  if(millis() - last > 5000) {
    last = millis();
    musician.play();
    Serial.println("Playing again");
  }
}

 const int chan = 0;
 double freq = 200;
void setup() {
  delay(200);
  Serial.begin(115200);
  Serial.println("Simple buzzer initiated");
 ledcAttachPin(PIN::HEALTH_CHECK_LED, chan);
  ledcWriteTone(chan, freq);
}

float ramp = 0.0002;
void loop() {
  if (millis() % 10 == 0) {
    freq = freq * (1 + ramp);
    ledcWriteTone(chan, freq);
  }
  if(freq > 10000 || freq < 100)
    ramp *= -1;
    // freq = 200;
}
