#pragma once

#include "coroutines.hpp"
#include "pins.hpp"
#include "logger.hpp"
#include "characters.hpp"

namespace Knobs {

using std::abs;
using std::sin;
using std::pow;

constexpr int N_INPUTS = 3;
constexpr int SENSITIVITY = 100;
using Inputs = int[N_INPUTS];
  
[[gnu::pure]] int _selector(const Inputs & i) {
  float x = i[0] / 100.0, y = i[1] / 100.0;
  float out = abs(sin(x * y) * 1.2 + sin(pow(y, 1.2)));
  out = out * 100 / 2.2;
  return out;
}

struct Digit{
  const int data_pin = -1;
  const int clock_pin = -1;

  union state {
    int number;
  } state;
  void setup() {
    pinMode(data_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
  }
  void write(int number) {
    assert(number >= 0 && number < 100);
    state.number = number;
    int d0 = number % 10,
      d1 = (number / 10) % 10;
    d0 = get_7_seg_from_int(d0);
    d1 = get_7_seg_from_int(d1);
    shiftOut(data_pin, clock_pin, false, d0);
    shiftOut(data_pin, clock_pin, false, d1);
  }

} digit_display;

struct Knobs: BaseCoroutine {
  
  Inputs last_inputs;
  Inputs input_pins {}; // TODO: add pins
  
  int runCoroutine() override {
    COROUTINE_LOOP() {
      Inputs new_inputs;
      int cum_difference = 0;
      for(int i=0; i<N_INPUTS; ++i){
        int new_input = analogRead(input_pins[i]);
        cum_difference += abs(new_input - last_inputs[i]);
        last_inputs[i] = new_input;
      }
      if (cum_difference < SENSITIVITY) {
        COROUTINE_DELAY(10);
      } else {
        int selector_out = _selector(new_inputs);
        digit_display.write(selector_out);
      }
    }
  }
  void setupCoroutine() {
    digit_display.setup();
  }
  
  enum Mode {
    MODE1, MODE2, MODE3, MODE4
  } mode;
  
  void set_mode(int s) {
    if(s < 10)
      mode = MODE1;
    else if (s < 20)
      mode = MODE2;
    else if (s < 30)
      mode = MODE3;
    else 
      mode = MODE4;
  }

  
} knobs;

}
