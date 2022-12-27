#include <String>
#include <Arduino.h>

#define NO_DITHERING 1
#include <FastLED.h>
#undef NO_DITHERING

#include <AceRoutine.h>
#include <AceButton.h>
#include <Melody.h>
#include <Musician.h>
#include <Preferences.h>

#define DEBUGGING 1

#include "utils.hpp"
#include "logger.hpp"
#include "led.cpp"
#include "pins.hpp"
#include "coroutines.hpp"

#include "serial_button.hpp"



constexpr bool PROFILE = false;


using ace_routine::CoroutineScheduler;
using namespace ace_button;
using namespace ace_routine;



constexpr uint8_t
  LEDC_CHANNEL_BUZZER = 0;

// OUTPUTS

// DATA
struct Data {
  Musician buzzer_musician = Musician( PIN::BUZZER, LEDC_CHANNEL_BUZZER );


  void setup() = delete; // data needs to be dumb


} data;



// INPUTS

struct Inputs { // convert to namespace and globals?
  // bool _m;
  void setup() {
  }
  void loop() {
  }
} input;


// Helpers
namespace {



}



////////////// COROUTINES ////////////////

COROUTINE(read_inputs) {
  COROUTINE_LOOP() {
    input.loop();
    COROUTINE_DELAY(5);
  }
}

namespace HealthCheckLed {
Led onboard_led = Led{PIN::HEALTH_CHECK_LED};
class HealthCheckLed : Coroutine {
  int runCoroutine() override {
    COROUTINE_LOOP() {
      onboard_led.toggle();
      COROUTINE_DELAY(500);
      DEBUG_SLOW("HealthCheck");
    }
  }
  
  void setupCoroutine() override {
    onboard_led.setup();
    setName("health_check_led");
  }
} health_check_led;
}





COROUTINE(printProfiling) {
  COROUTINE_LOOP() {
    if(!PROFILE) {
      COROUTINE_END();
    }
    LogBinTableRenderer::printTo(
        Serial, 3 /*startBin*/, 14 /*endBin*/, false /*clear*/);
    COROUTINE_DELAY(10000);
  }
}

// #include "buzzer.hpp"
#include "whack_mole.hpp"
#include "led_matrix.hpp"
#include "strip_bounce.hpp"
#include "led_control.hpp"

// END COROUTINES



void dump() {
}


void set_coroutine_names() {
  #ifdef DEBUGGING
  read_inputs.setName("read_inputs");
  // StripBounce::strip_bounce.setName("strip_show_leds");
  printProfiling.setName("print_profiling");
  #endif
}

void treat_serial() {
  String input = Serial.readStringUntil('\n');
  switch(input[0]) {
  case('!'):
    preferences.write(input.substring(1)); break;
  case('D'):
    dump(); break;
  case('W'):
    Serial << "Moles:" << ARR(WhackMole::mole_present) << endl;
    WhackMole::button_config.refresh();
    Serial << ARR(WhackMole::button_config.last_reading) << endl;
    break;
  default:
    Serial << "Valid commands are '!' 'D' 'W' . " << endl;
  }
}



// Arduino CODE

/*/
#define     HELINDAO_BOARD_TEST
//*/
#ifndef     HELINDAO_BOARD_TEST

constexpr long MILLION = 1000000;

void _light_sleep() {
  DEBUG("Sleeping..."); delay(10);
  esp_sleep_enable_timer_wakeup(5 * MILLION);
  DEBUG(esp_light_sleep_start());
  DEBUG("Woke!");
}

void setup() {
  Serial.begin(115200);
  Serial.println("Alive");
  delay(500);  // Maybe this helps upload success on boot?
  // _light_sleep();
  input.setup();
  preferences.setup();

  set_coroutine_names();
  CoroutineScheduler::setup();
  CoroutineScheduler::setupCoroutines();
  if(PROFILE)
    LogBinProfiler::createProfilers();
  randomSeed(micros());
}

void loop() {
  if(PROFILE)
    CoroutineScheduler::loopWithProfiler();
  else
    CoroutineScheduler::loop();
  if (Serial.available() > 0) {  
    treat_serial();
  }
}

#else
#include "standalone_tests/musician_buzzer.hpp"
#endif
