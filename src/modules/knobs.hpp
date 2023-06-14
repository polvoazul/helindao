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
  
// Inputs are from 0 to 100 and outputs is a number from 0 to 99
[[gnu::pure]] int16_t _selector(const Inputs & i) {
  float x = i[0] / 100.0, y = i[1] / 100.0, z = i[2] / 100.0;
  float out = abs( sin(x * y) * 1.2 + sin(pow(y, 1.2)) + sin(x)/(abs(10*y) + 0.5) + z);
  out = out * 100 / 4.0;
  return out;
}

struct Digit {
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
      int cum_difference = 0;
      for(int i=0; i<N_INPUTS; ++i){
        int new_input = analogRead(input_pins[i]);
        cum_difference += abs(new_input - last_inputs[i]);
        last_inputs[i] = new_input;
      }
      if (cum_difference > SENSITIVITY) {
        int16_t selector_out = _selector(last_inputs);
        digit_display.write(selector_out);
        set_mode(selector_out);
      } else {
        COROUTINE_DELAY(100);
      }
      COROUTINE_DELAY(30);
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
    else // 30 < s < 100
      mode = MODE4;
  }

  
} knobs;

}
