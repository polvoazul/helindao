#include <Arduino.h>
#include <FastLED.h>
#include <AceRoutine.h>
#include <AceButton.h>
#include <Melody.h>
#include <Musician.h>

#define DEBUGGING 1
#include "logger.hpp"

#include "led.cpp"
#include "pins.hpp"
#include "coroutines.hpp"




constexpr bool PROFILE = false;


using ace_routine::CoroutineScheduler;
using namespace ace_button;
using namespace ace_routine;




constexpr uint8_t
  LEDC_CHANNEL_BUZZER = 0,
  WHACK_MOLE_N = 3;

// OUTPUTS
struct Outputs {
  Led whack_mole_leds[3] = {PIN::WHACK_MOLE_LED1, PIN::WHACK_MOLE_LED2, PIN::WHACK_MOLE_LED3};

  void setup() {
    // Setup Leds array
    for(Led & led : whack_mole_leds) led.setup();
  }
} output;

// DATA
struct Data {
  Musician buzzer_musician = Musician( PIN::BUZZER, LEDC_CHANNEL_BUZZER );


  void setup() = delete; // data needs to be dumb


} data;



// INPUTS

struct Inputs { // convert to namespace and globals?
  ButtonConfig _whack_mole_config;
  AceButton whack_mole_button[3];

  Inputs()
  {
    whack_mole_button[0].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON1, HIGH, 0);
    whack_mole_button[1].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON2, HIGH, 1);
    whack_mole_button[2].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON3, HIGH, 2);
  }


  void setup() {
    // Strip

    // Mole
    for (auto & button : whack_mole_button) {
      pinMode(button.getPin(), INPUT_PULLUP);
    }

  }

  void loop() {
    for (auto &button: whack_mole_button)
      button.check();
  }

} input;


// Helpers
namespace {


inline void _print_fps() {
    static int last_second = 0;
    static int frames_since_last_second = 0;

    ++frames_since_last_second;
    int now = millis() / 1000;
    if (now == last_second) return;
    Serial.print("FPS: ");
    Serial.println(frames_since_last_second);
    last_second = now;
    frames_since_last_second = 0;
}

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

// #include "whack_mole.hpp"
// #include "buzzer.hpp"
// #include "led_matrix.hpp"
#include "./strip_bounce.hpp"

// END COROUTINES



void dump() {
  StripBounce::dump();
}


void set_coroutine_names() {
  #ifdef DEBUGGING
  read_inputs.setName("read_inputs"); StripBounce::show_leds.setName("strip_show_leds");
  printProfiling.setName("print_profiling");
  #endif
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
  output.setup();
  input.setup();

  set_coroutine_names();
  CoroutineScheduler::setup();
  CoroutineScheduler::setupCoroutines();
  if(PROFILE)
    LogBinProfiler::createProfilers();
}

void loop() {
  if(PROFILE)
    CoroutineScheduler::loopWithProfiler();
  else
    CoroutineScheduler::loop();
  if (Serial.available() > 0) {  
    while (Serial.available()) Serial.read();
    dump();
  }
}

#else
#include "standalone_tests/musician_buzzer.hpp"
#endif
