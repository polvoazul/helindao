#include "pins.hpp"
#include <Musician.h>
#include <Melody.h>

#pragma once


 Musician musician = Musician(PIN::HEALTH_CHECK_LED, 0);
  const char melody_simple[] = "((cge+)**)<<<";
  const char darthVader[] = " (ggg e,-. b,-- | g e,-. b,-- g+ (ddde,-.)* b,--  | g, e,-. b,-- g+ | g* g-.g--  (g g,-. f-- (ed#)-- e-)* r- g#- c#* b#-.b-- |  (b,a)-- b,- r- e,- g, e,-. g,-- | b, g-. b,-- d*+  | g* g-.g--  (g g,-. f-- (ed#)-- e-)* r- g#- c#* b#-.b-- |  (b,a)-- b,- r- e,- g, e,-. b,-- | g e,-. b,-- g+ |)<<_* ";
  const char frereJacques[] = "((cdec)x2   (efgr)x2   ((gagf)-ec)x2     (c g_ c+)x2)*";


 Melody melody;
void setup() {
  delay(200);
  Serial.begin(115200);
  Serial.println("Simple buzzer initiated");

  melody.setScore(frereJacques);
  melody.setTempo(200);
  musician.setLoudnessLimit(128, 128);
  musician.setMelody(&melody);
  musician.setBreath(90);
  musician.play();
  musician.refresh();
}

void loop() {
  musician.refresh();
  static int last = 0;
  if(millis() - last > 5000 && !musician.isPlaying()) {
    last = millis();
    musician.play();
    Serial.println("Playing again");
  }
}
