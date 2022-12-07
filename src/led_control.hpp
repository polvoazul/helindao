#pragma once
#include "coroutines.hpp"


// Unfortunatly we need to call each of this showLeds at the same time (i would like to call each in their own subroutine) 
// https://github.com/FastLED/FastLED/issues/1184

inline void _print_fps() {
    static int last_second = 0;
    static int frames_since_last_second = 0;

    ++frames_since_last_second;
    int now = millis() / 1000;
    if (now == last_second) return;
    Serial.print("FPS: ");
    Serial.println(frames_since_last_second);
    last_second = now;
    frames_since_last_second = 0;
}

constexpr int FPS = 30;

COROUTINE(show_leds) {
  COROUTINE_LOOP() {
    assert(StripBounce::SPEED < FPS);
    constexpr int delay = 1000 / FPS;
    _print_fps();
    if(
      StripBounce::refresh_needed
      #ifdef HELINDAO_LEDMATRIX
      || LedMatrix::refresh_needed
      #endif
      ) {
      StripBounce::controller->showLeds(Brightness(StripBounce::BRIGHTNESS));
      StripBounce::refresh_needed = false;
      #ifdef HELINDAO_LEDMATRIX
      LedMatrix::controller->showLeds(Brightness(LedMatrix::BRIGHTNESS));
      LedMatrix::refresh_needed = false;
      #endif
    }
    COROUTINE_DELAY(delay);
  }
}
