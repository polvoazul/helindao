#pragma once
#include <Arduino.h>
#include <AceRoutine.h>
#include <Melody.h>

namespace Buzzer {
// const char melody_s[] = "cde | f ff cd c | d dd cgf | e ee cde | f ff";
// const char melody_s[] = "cde | f ff cd c | d dd cgf | e ee cde | a"; // Crazy bug hang on long melody
// Melody melody_valid(" (cgc*)**---");  // Valid choice kind-of sound

Melody melody_invalid(" (cg_)__");  //Invalid choice kind-of sound
static const char melody_simple[] = "(cge+)*";
Melody melody(melody_simple);

struct BuzzerTone: public Coroutine {

  void setupCoroutine() override {
    melody.setScore(melody_simple);
    melody.setTempo(200);
    data.buzzer_musician.setMelody(&melody_invalid);
    data.buzzer_musician.setBreath(90);
    data.buzzer_musician.play();
    data.buzzer_musician.refresh();
  }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      if (data.buzzer_musician.isPlaying()) {
        data.buzzer_musician.refresh();
      }
      COROUTINE_DELAY(3);
    }
  }
} buzzer_tone;
}
