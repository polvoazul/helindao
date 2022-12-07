#include <Arduino.h>
#include <FastLED.h>
#include <AceRoutine.h>
#include <AceButton.h>

#include "pins.hpp"
#include "logger.hpp"
#include "coroutines.hpp"
#include "utils.hpp"

Print &operator<<(Print &s, CRGB const &rgb) { return s << "(" << rgb.r << ", " << rgb.g << ", " << rgb.g << ")";}

namespace StripBounce {
constexpr int STRIP_N = 29; // Speed is in jumps per second
const float & BRIGHTNESS = preferences.sb_brightness;
const int & SPEED = preferences.sb_speed;

CRGB strip[STRIP_N];
CLEDController *controller;
uint8_t scores[STRIP_N] = {};
uint8_t current_x = 0;
ButtonConfig _hit_config;
AceButton hit {&_hit_config, PIN::HIT_BUTTON, HIGH, 0};
bool refresh_needed;

static void handle_hit(AceButton* button, uint8_t event_type, uint8_t buttonState) {
  if(event_type != AceButton::kEventPressed) return;
  scores[current_x] += 60;
  // data.buzzer_musician.getMelody()->restart();
  // data.buzzer_musician.stop();
  // data.buzzer_musician.play();
  // data.buzzer_musician.refresh();
  DEBUG << "HIT: " << current_x << " -> " << scores[current_x] << endl;
}

void dump() {
  int i=0;
  for (int s : scores) {
    DEBUG << "Score " << i++ << " - " << s << endl;
  }
}

struct StripBounce : Coroutine {

  void setupCoroutine() override {
    setName("strip_bounce"); 
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 100); 
    controller = &FastLED.addLeds<NEOPIXEL, PIN::LED_STRIP_DATA>((CRGB*)strip, STRIP_N);
    controller->setDither(0);
    pinMode(PIN::HIT_BUTTON, INPUT_PULLUP); // Check if PULLUP works on this pin
    _hit_config.setEventHandler(&handle_hit);
  }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      const int DELAY = 1000 / SPEED,
        SATURATION = 200;
      constexpr uint8_t SCORE_DECAY = 3;
      static int8_t direction = 1;
      static float hue = 0;
      constexpr float HUE_SPEED = 0.7;

      hue += HUE_SPEED;
      uint8_t &score = scores[current_x];
      strip[current_x].setHSV((uint8_t)hue, SATURATION, score);
      score = score > SCORE_DECAY ? score - SCORE_DECAY : 0;
      switch (current_x) {
        case 0: direction = 1; break;
        case STRIP_N - 1: direction = -1; break;
      }
      current_x += direction;
      strip[current_x] = CRGB::DarkRed;
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

} // namespace StripBounce
