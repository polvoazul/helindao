#pragma once
#include "coroutines.hpp"


// Unfortunatly we need to call each of this showLeds at the same time (i would like to call each in their own subroutine) 
// https://github.com/FastLED/FastLED/issues/1184

inline void _print_fps() {
    static int last_second = 0;
    static int frames_since_last_second = 0;
    constexpr int CALCULATE_EACH = 5;

    ++frames_since_last_second;
    int now = millis() / (1000 * CALCULATE_EACH);
    if (now == last_second) return;
    DEBUG << "FPS: " << frames_since_last_second / ((float)CALCULATE_EACH) << endl;
    last_second = now;
    frames_since_last_second = 0;
}

constexpr int FPS = 30;

COROUTINE(show_leds) {
  COROUTINE_LOOP() {
    assert(StripBounce::SPEED < FPS);
    constexpr int delay = 1000 / FPS;
    _print_fps();
    if( false
      #ifdef HELINDAO_STRIP_BOUNCE
      || StripBounce::refresh_needed
      #endif
      #ifdef HELINDAO_LED_MATRIX
      || LedMatrix::refresh_needed
      #endif
      ) {
      #ifdef HELINDAO_STRIP_BOUNCE
      StripBounce::controller->showLeds(Brightness(StripBounce::BRIGHTNESS));
      StripBounce::refresh_needed = false;
      #endif
      #ifdef HELINDAO_LED_MATRIX
      LedMatrix::controller->showLeds(Brightness(LedMatrix::BRIGHTNESS));
      LedMatrix::refresh_needed = false;
      #endif
    }
    COROUTINE_DELAY(delay);
  }
}
