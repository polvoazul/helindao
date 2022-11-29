#pragma once
#include <Arduino.h>
#include <AceRoutine.h>
#include <AceButton.h>

namespace WhackMole {
bool whack_mole_present[WHACK_MOLE_N];
int whack_mole_score;
enum State {
  ON,
  WINNING
} state = State::ON;
constexpr int WINNING_BLINKS_N = 3;

struct WhackMole : public Coroutine {
  static constexpr int INITIAL_MOLES = 1;

  static void handle_whack_button(AceButton* button, uint8_t event_type, uint8_t buttonState){
    if(state == WINNING) return;
    if(event_type != AceButton::kEventPressed) return;
    int whacked_mole = button->getId();
    if(!whack_mole_present[whacked_mole]) return;
    // whacked!
    set_mole(whacked_mole, false);
    int next = random() % WHACK_MOLE_N - 1;
    if(next == whacked_mole) next = WHACK_MOLE_N - 1;
    set_mole(next);
    whack_mole_score += 1;
    if(whack_mole_score > 10)
      state = WINNING;
  }

  static void set_mole(int i, bool on=true) {
    assert(i < WHACK_MOLE_N);
    whack_mole_present[i] = on;
    output.whack_mole_leds[i].set(on); // move to output loop?
  }


  void setupCoroutine() override {
    for(bool & mole : whack_mole_present)
      mole = false;
    whack_mole_score = 0;
    input._whack_mole_config.setEventHandler(&handle_whack_button);
    for(int i=0; i<INITIAL_MOLES; i++) {
      int mole = random() % WHACK_MOLE_N;
      set_mole(mole);
    }
  }

  void set_all_whack_leds(bool on = true) {
    for(auto & l : output.whack_mole_leds)
      l.set(on); // move to output loop?
  }


  int winning_blinks = 0;  // corroutine local variables need to be stored in the object for persistance
  int runCoroutine() override {
    COROUTINE_LOOP() {
      COROUTINE_AWAIT(state == WINNING); // Change to launch task?
      for(winning_blinks=0; winning_blinks < WINNING_BLINKS_N; winning_blinks++) {
        set_all_whack_leds(true);
        COROUTINE_DELAY_SECONDS(1);
        set_all_whack_leds(false);
        COROUTINE_DELAY_SECONDS(1);
      }
      state = ON;
      whack_mole_score = 0;
    }
  }
} whack_mole;
}
