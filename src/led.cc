#include <Arduino.h>

class Led {
private:
  uint8_t pin;
public:
  Led(uint8_t pin) : pin(pin) {}
  // Led(const Led & pin) = delete;
  // Led(Led && pin) = delete;
  void setup() {
    pinMode(pin, OUTPUT);
    off();
  }
  void on() {
    digitalWrite(pin, HIGH);
  }
  void off() {
    digitalWrite(pin, LOW);
  }
};
