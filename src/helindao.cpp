#include <Arduino.h>
#include <FastLED.h>
#include <AceRoutine.h>
#include <AceButton.h>
#include <Melody.h>
#include <Musician.h>

#include "logger.hpp"
#include "led.cpp"
#include "pins.hpp"


#define LOG(x) Serial.println((x))
#define DEBUG(x) LOG(x)

#define TOKEN_PASTE(x, y) x##y
#define CAT(x,y) TOKEN_PASTE(x,y)
#define UNIQUE_INT CAT(__debug_slow, __LINE__)
#define DEBUG_SLOW(x) static ulong UNIQUE_INT; if (millis() - UNIQUE_INT > 5000) {LOG(x); UNIQUE_INT = millis();}

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

constexpr uint8_t Brightness(float b) {return static_cast<uint8_t>(b*255);}

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

namespace LedMatrix {
  #include "./matrix_images/8x8_characters.cpp"
  constexpr int FPS = 40;
  constexpr int delay = 1000 / FPS;
  constexpr auto BRIGHTNESS = Brightness(0.05);
  static constexpr int MATRIX_N = 8;
  CRGB matrix[MATRIX_N][MATRIX_N];
  CLEDController *controller;
  bool refresh_needed = true;

  struct LedMatrix : public Coroutine {

  void setupCoroutine() override {
    setName("led_matrix");
    controller = &FastLED.addLeds<NEOPIXEL, PIN::MATRIX_LED_DATA>((CRGB*)matrix, MATRIX_N*MATRIX_N);
  }

  int runCoroutine() override {
    COROUTINE_BEGIN();
    COROUTINE_DELAY_SECONDS(5);
    DEBUG("Running Matrix");
    while(true) {
      if(refresh_needed) {
        controller->showLeds(BRIGHTNESS);
        refresh_needed = false;
      }
      COROUTINE_DELAY(delay);
    }
  }
  
  } led_matrix;

  COROUTINE(SwapImage) {
    constexpr int SWAP_IMAGE_TIME_SECONDS = 15;
    COROUTINE_LOOP() {
      const int next_image = 0;// random() % 100; // N_IMAGES = 100
      const unsigned char (&image)[8][8][3] = images8x8_characters[next_image];
      for (int y=0; y<8; ++y) {  // OPT: Copy memory in chunks
        for (int x=0; x<8; ++x) {
          matrix[y][x] = CRGB{image[y][x][0], image[y][x][1], image[y][x][2]};
        }
      }
      DEBUG("Image chosen: "); 
      DEBUG(next_image); 
      refresh_needed = true;
      COROUTINE_DELAY_SECONDS(SWAP_IMAGE_TIME_SECONDS);
    }
  }
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
#include "./strip_bounce.hpp"

// END COROUTINES



void dump() {
  StripBounce::dump();
}


void set_coroutine_names() {
  read_inputs.setName("read_inputs"); StripBounce::show_leds.setName("strip_show_leds");
  printProfiling.setName("print_profiling"); LedMatrix::SwapImage.setName("swap_image");
}



// Arduino CODE

/*/
#define     HELINDAO_BOARD_TEST
//*/
#ifndef     HELINDAO_BOARD_TEST

constexpr long MILLION = 1000000;

void _light_sleep() {
  DEBUG("Sleeping..."); delay(10);
  esp_sleep_enable_timer_wakeup(1 * MILLION);
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
