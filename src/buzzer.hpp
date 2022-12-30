#pragma once
#include <Arduino.h>
#include <Melody.h>
#include <Musician.h>

#include "coroutines.hpp"
#include "pins.hpp"

namespace Buzzer {

namespace Sound {

constexpr char
  melody1[] = "(cde | f ff cd c | d dd cgf | e ee cde | f ff)-",
  melody2[] = "(cde | f ff cd c | d dd cgf | e ee cde | a)-",
  effect_valid[] = "(cgc*)",  // Valid choice kind-of sound
  effect_invalid[] = "(cg_)_",    // Invalid choice kind-of sound
  simple[] = "(cge+)*";
}

Melody melody;
Musician musician {.address = PIN::BUZZER, .channel = PIN::LEDC::BUZZER};

void play(const char * score) {
  melody.setScore(score); // Maybe pre-cache melodies instead of strings?
  melody.restart();
  musician.setMelody(&melody);
  musician.play();
  musician.refresh();
}

struct BuzzerTone: public BaseCoroutine {
  void setupCoroutine() override {
    melody.setTempo(180);
    musician.setBreath(90);
    musician.setLoudnessLimit(128, 128);
    play(Sound::simple);
  }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      if (musician.isPlaying()) {
        musician.refresh();
      }
      COROUTINE_DELAY(3);
    }
  }
} buzzer_tone;
} // namespace Buzzer
