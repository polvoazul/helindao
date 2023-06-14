#pragma once

#include "coroutines.hpp"
#include "pins.hpp"
#include "logger.hpp"

#include <cmath>

namespace Motors {

int avg;

struct MotorOutput : BaseCoroutine {
  int8_t speed;
  bool changed;

  void setupCoroutine() {
    pinMode(PIN::MOTOR_OUT, OUTPUT);
  }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      if (changed) {
        changed = false;
        analogWrite(PIN::MOTOR_OUT, speed);
      } else {
        COROUTINE_DELAY(100);
      }
    }
  }
  
  void set_speed(float _speed) {
    speed = static_cast<int>(255*_speed);
    changed = true;
  }
} motor_output;

struct MotorInput : BaseCoroutine {

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
        COROUTINE_DELAY(5);
      }
    }
  }

  #undef calibrate


  // motor_velocity is a signed int, sign is the direction of rotation
  void handle_action(int motor_velocity) {
    DEBUG << "Motor movement detected, with velocity: " << motor_velocity << endl;
    motor_output.set_speed(std::abs(motor_velocity/200));
  }
} motor_input;



} // namespace Motors
