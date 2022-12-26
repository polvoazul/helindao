#pragma once
#include <Arduino.h>
#include <AceButton.h>

template <int NUMBER_OF_BUTTONS>
struct SerialButton : ace_button::ButtonConfig {
  bool last_reading[NUMBER_OF_BUTTONS];
  uint8_t load_pin, data_pin, clock_pin;

  SerialButton(uint8_t load_pin, uint8_t data_pin, uint8_t clock_pin) :
      load_pin(load_pin), data_pin(data_pin), clock_pin(clock_pin) {
      for(auto & button : last_reading) 
        button = HIGH; // default for buttons is to be pulled up at high
    }
  
  void setup() {
    pinMode(load_pin, OUTPUT);
    pinMode(clock_pin, OUTPUT);
    pinMode(data_pin, INPUT);
    digitalWrite(load_pin, HIGH);  // Load pin is inverted, it loads on LOW
    digitalWrite(clock_pin, LOW);
  }

  // Need to call this to refresh whole button array state. Always call before calling readButton (button.check())
  void refresh() { 
    digitalWrite(load_pin, LOW);  // Load pin is inverted, it loads on LOW
    digitalWrite(clock_pin, HIGH);
    digitalWrite(load_pin, HIGH);
    digitalWrite(clock_pin, LOW);
    for(int i=0; i<NUMBER_OF_BUTTONS; i++) {
      digitalWrite(clock_pin, HIGH);
      last_reading[i] = digitalRead(data_pin);
      digitalWrite(clock_pin, LOW);
    }
  }
 
  int readButton(uint8_t pin) override {
    pin = pin - 100; // using virtual pin numbers to make sure we fail fast if naive numbers are provided
    assert(0 <= pin && pin < NUMBER_OF_BUTTONS);
    return last_reading[pin];
  }
};

template <uint NUMBER_OF_OUTPUTS>
struct SerialOutput {
  bool outputs[NUMBER_OF_OUTPUTS] {};
  bool refresh_needed;

  uint8_t data_pin, clock_pin;

  SerialOutput(uint8_t data_pin, uint8_t clock_pin) :
      data_pin(data_pin), clock_pin(clock_pin) {}

  void setup() {
    pinMode(clock_pin, OUTPUT);
    pinMode(data_pin, OUTPUT);
    digitalWrite(data_pin, LOW);
    digitalWrite(clock_pin, LOW);
  }

  // Set a value but doesn't flush it to the hardware.
  inline void set(uint8_t i, bool value) {
    outputs[i] = value;
    refresh_needed = true;
  }

  // Flush values to hardware.
  void write() {
    if(!refresh_needed) return;
    digitalWrite(clock_pin, LOW);
    for(int i=0; i<NUMBER_OF_OUTPUTS; i++) {
      digitalWrite(data_pin, outputs[i]);
      digitalWrite(clock_pin, HIGH);
      digitalWrite(clock_pin, LOW);
      #ifdef HELINDAO_SERIAL_BUTTON_DEBUG_SLOW
      delay(1000);
      #endif
    }
    refresh_needed = false;
  }
};
