#include <Arduino.h>

struct Led {
  uint8_t pin;
  bool state;

  Led(uint8_t pin) : pin(pin) {}
  // Led(const Led & pin) = delete;
  // Led(Led && pin) = delete;
  void setup() {
    pinMode(pin, OUTPUT);
    off();
  }
  void on() {
    state = true;
    digitalWrite(pin, HIGH);
  }
  void off() {
    state = false;
    digitalWrite(pin, LOW);
  }
  void toggle() {
    set(!state);
  }
  void set(bool on) {
    if(on) this->on(); else off();
  }
};
