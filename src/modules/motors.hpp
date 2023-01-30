#pragma once

#include "coroutines.hpp"
#include "pins.hpp"
#include "logger.hpp"

namespace Motors {

int avg;

struct Motors : BaseCoroutine {

  void setupCoroutine() {
      pinMode(PIN::MOTOR_READ_PIN, INPUT);
  }

  #define calibrate() \
    constexpr int INIT_SAMPLES = 100; \
      avg = 0; \
      for (i = 0; i < INIT_SAMPLES; ++i) {\
        uint motor_input = analogRead(PIN::MOTOR_READ_PIN); \
        avg += motor_input; \
        COROUTINE_DELAY(10); \
      } \
      avg = avg / i; \
      DEBUG << "Calibration of motor completed. Found value: " << avg << endl; \
  
  int i;
  int runCoroutine() override {
    constexpr int SENSITIVITY = 100;
    COROUTINE_BEGIN() { 
      calibrate();
      while (true) {
        int motor_input = avg - analogRead(PIN::MOTOR_READ_PIN);
        if (motor_input > SENSITIVITY || motor_input < -SENSITIVITY)
          handle_action(motor_input);
        COROUTINE_DELAY(3);
      }
    }
  }

  #undef calibrate


  // motor_velocity is a signed int, sign is the direction of rotation
  void handle_action(int motor_velocity) {
    DEBUG << "Motor movement detected, with velocity: " << motor_velocity << endl;
  }
} motor;




} // namespace Motors
