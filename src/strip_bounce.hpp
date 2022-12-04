#include <Arduino.h>
#include <FastLED.h>
#include <AceRoutine.h>
#include <AceButton.h>

#include "pins.hpp"
#include "logger.hpp"

Print &operator<<(Print &s, CRGB const &rgb) { return s << "(" << rgb.r << ", " << rgb.g << ", " << rgb.g << ")";}

namespace StripBounce {
constexpr int STRIP_N = 29;

CRGB strip[STRIP_N];
CLEDController *strip_controller;
uint8_t strip_bounce_score[STRIP_N] = {};
uint8_t strip_bounce_current_x = 0;
ButtonConfig _hit_config;
AceButton hit {&_hit_config, PIN::HIT_BUTTON, HIGH, 0};
bool refresh_needed;

static void handle_hit(AceButton* button, uint8_t event_type, uint8_t buttonState) {
  if(event_type != AceButton::kEventPressed) return;
  strip_bounce_score[strip_bounce_current_x] += 60;
  // data.buzzer_musician.getMelody()->restart();
  // data.buzzer_musician.stop();
  // data.buzzer_musician.play();
  // data.buzzer_musician.refresh();
  Serial.print("HIT: ");
  Serial.print(strip_bounce_current_x);
  Serial.print(" -> ");
  Serial.println(strip_bounce_score[strip_bounce_current_x]);
}

void dump() {
  int i=0;
  for (int s : strip_bounce_score) {
    Serial.print("Score "); Serial.print(i++); Serial.print(" - "); Serial.println(s);
  }
}

struct StripBounce : Coroutine {

  void setupCoroutine() override {
    setName("strip_bounce"); 
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 100); 
    strip_controller = &FastLED.addLeds<NEOPIXEL, PIN::LED_STRIP_DATA>((CRGB*)strip, STRIP_N);
    pinMode(PIN::HIT_BUTTON, INPUT_PULLUP); // Check if PULLUP works on this pin
    _hit_config.setEventHandler(&handle_hit);
  }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      constexpr int SPEED = 10,
        DELAY = 1000 / SPEED,
        SATURATION = 200;
      constexpr uint8_t SCORE_DECAY = 3;
      static int8_t direction = 1;
      static float hue = 0;
      constexpr float HUE_SPEED = 0.7;

      hue += HUE_SPEED;
      auto *score = &strip_bounce_score[strip_bounce_current_x];
      strip[strip_bounce_current_x].setHSV((uint8_t)hue, SATURATION, *score);
      *score = *score > SCORE_DECAY ? *score - SCORE_DECAY : 0;
      switch (strip_bounce_current_x) {
        case 0: direction = 1; break;
        case STRIP_N - 1: direction = -1; break;
      }
      strip_bounce_current_x += direction;
      strip[strip_bounce_current_x] = CRGB::Red;
      refresh_needed = true;
      COROUTINE_DELAY(DELAY);
    }
  }
} strip_bounce;

COROUTINE(check_hit_button) {
  COROUTINE_LOOP() {
    hit.check();
    COROUTINE_DELAY(4);
  }
}

void Print(CRGB pixel) {

}

COROUTINE(show_leds) {
  COROUTINE_LOOP() {
    constexpr int FPS = 30;
    constexpr int delay = 1000 / FPS;
    constexpr auto BRIGHTNESS = Brightness(0.10);
    _print_fps();
    if(refresh_needed) {
      strip_controller->showLeds(BRIGHTNESS);
      refresh_needed = false;
      int c = 0;
      Serial.println(millis());
      Serial << ARR(strip) << endl;
    }
    COROUTINE_DELAY(delay);
  }
}

}
