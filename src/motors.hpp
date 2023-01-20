#pragma once

#include "coroutines.hpp"
#include "pins.hpp"

namespace Motors {

int avg;

struct Motors : BaseCoroutine {

  void setupCoroutine() {
      pinMode(PIN::MOTOR_READ_PIN, INPUT);
  }
  
  int i;
  int runCoroutine() override {
    constexpr uint SENSITIVITY = 100;
    COROUTINE_BEGIN() { 
      avg = 0;
      for(i=0;i<100;++i) {
        uint motor_input = analogRead(PIN::MOTOR_READ_PIN);
        avg += motor_input;
        COROUTINE_DELAY(10);
      }
      avg = avg / i;
      Serial << "Calibration of motor completed. Found value: " << avg << endl;

      while(true) {
        int motor_input = avg - analogRead(PIN::MOTOR_READ_PIN);
        uint motor_power = std::abs(motor_input);
        if(motor_power > SENSITIVITY)
          handle_action(motor_power, motor_input > 0);
        COROUTINE_DELAY(3);
      }
    }
  }

  void handle_action(uint motor_power, bool direction) {

  }

} motor;




} // namespace Motors
