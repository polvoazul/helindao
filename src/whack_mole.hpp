#pragma once
#include <Arduino.h>
#include <AceRoutine.h>
#include <AceButton.h>
#include "serial_button.hpp"
#include "pins.hpp"


namespace WhackMole {

constexpr uint8_t 
  PIN_BUTTON_DATA_IN = PIN::WHACK_MOLE_DB,
  PIN_BUTTON_CLOCK   = PIN::WHACK_MOLE_DL_CB,
  PIN_BUTTON_LOAD    = PIN::WHACK_MOLE_LB,
  PIN_LED_CLOCK      = PIN::WHACK_MOLE_CL,
  PIN_LED_DATA_OUT   = PIN::WHACK_MOLE_DL_CB;

using ace_button::AceButton;

constexpr int WHACK_MOLE_N = 6;

bool whack_mole_present[WHACK_MOLE_N] {};
int whack_mole_score = 0;
enum State {
  ON,
  WINNING
} state {ON};
constexpr int WINNING_BLINKS_N = 3;

SerialOutput<WHACK_MOLE_N> serial_output {.data_pin = PIN_LED_DATA_OUT, .clock_pin = PIN_LED_CLOCK};
SerialButton<WHACK_MOLE_N> button_config {.load_pin = PIN_BUTTON_LOAD, .data_pin = PIN_BUTTON_DATA_IN, .clock_pin = PIN_BUTTON_CLOCK}; 
AceButton buttons[WHACK_MOLE_N];

constexpr int INITIAL_MOLES = 1;

void set_mole(int i, bool on=true) {
  assert(i < WHACK_MOLE_N);
  whack_mole_present[i] = on;
  serial_output.set(i, on);
}

void flush_leds() {
  serial_output.write();
}

void handle_whack_button(AceButton* button, uint8_t event_type, uint8_t buttonState){
  if(state == WINNING) return;
  if(event_type != AceButton::kEventPressed) return;
  int whacked_mole = button->getId();
  if(!whack_mole_present[whacked_mole]) return;
  // whacked!
  uint8_t next;
  do {
    next = random() % WHACK_MOLE_N;
  } while(whack_mole_present[next]);
  set_mole(whacked_mole, false);
  set_mole(next, true);
  whack_mole_score += 1;
  if(whack_mole_score > 10)
    state = WINNING;
  flush_leds();
}

void set_all_whack_leds(bool on = true) {
  for(int i=0; i<WHACK_MOLE_N; i++)
    serial_output.set(i, on); // move to output loop?
  flush_leds();
}

struct WhackMole : public ace_routine::Coroutine {
  void setupCoroutine() override {
    button_config.setEventHandler(&handle_whack_button);
    for (int i=0; i<WHACK_MOLE_N; i++)
      buttons[i].init(&button_config, /* virtual pin */ i + 100, HIGH, i);
    for(int i=0; i<INITIAL_MOLES; i++) {
      int mole;
      do {
        mole = random() % WHACK_MOLE_N;
      } while (whack_mole_present[mole]);
      set_mole(mole);
    }
    flush_leds();
  }


  int winning_blinks = 0;  // corroutine local variables need to be stored in the object for persistance
  int runCoroutine() override { COROUTINE_LOOP() { switch(state) {
    case State::ON:
      button_config.refresh();
      for(auto & button : buttons)
        button.check();
      COROUTINE_DELAY(4);
      break;
    case State::WINNING:
      for(winning_blinks=0; winning_blinks < WINNING_BLINKS_N; winning_blinks++) {
        set_all_whack_leds(true);
        COROUTINE_DELAY_SECONDS(1);
        set_all_whack_leds(false);
        COROUTINE_DELAY_SECONDS(1);
      }
      state = ON;
      whack_mole_score = 0;
      break;
    default:
      assert(false);
  } } }

} whack_mole;
}
