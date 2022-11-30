#include <Arduino.h>
#include <FastLED.h>
#include <AceRoutine.h>
#include <AceButton.h>
#include <Melody.h>
#include <Musician.h>

#include "led.cpp"
#include "pins.hpp"


#define LOG(x) Serial.println((x))
#define DEBUG(x) LOG(x)

constexpr bool PROFILE = false;


using ace_routine::CoroutineScheduler;
using namespace ace_button;
using namespace ace_routine;




constexpr uint8_t
  LEDC_CHANNEL_BUZZER = 0,
  WHACK_MOLE_N = 3;

// OUTPUTS
struct Outputs {
  static constexpr int STRIP_N = 29;
  static constexpr int MATRIX_N = 8;
  // CRGB matrix[MATRIX_N][MATRIX_N];
  CRGB strip[STRIP_N];
  Led whack_mole_leds[3] = {PIN::WHACK_MOLE_LED1, PIN::WHACK_MOLE_LED2, PIN::WHACK_MOLE_LED3};

  void setup() {
    // Setup Leds array
    // constexpr EOrder RGB_ORDER = EOrder::RGB;
    // FastLED.addLeds<LED_MODEL>((CRGB*)matrix, MATRIX_N * MATRIX_N);
    FastLED.addLeds<NEOPIXEL, PIN::LED_STRIP_DATA>((CRGB*)strip, STRIP_N);
       FastLED.setMaxPowerInVoltsAndMilliamps(5,100); 


    for(Led & led : whack_mole_leds) led.setup();
  }
} output;

// DATA
struct Data {
  uint8_t strip_bounce_score[output.STRIP_N] = {};
  uint8_t strip_bounce_current_x = 0;
  Musician buzzer_musician = Musician( PIN::BUZZER, LEDC_CHANNEL_BUZZER );


  void setup() = delete; // data needs to be dumb

  void dump() {
    int i=0;
    for (int s : strip_bounce_score) {
      Serial.print("Score "); Serial.print(i++); Serial.print(" - "); Serial.println(s);
    }
  }

} data;



// INPUTS

struct Inputs { // convert to namespace and globals?
  ButtonConfig _hit_config, _whack_mole_config;
  AceButton hit;
  AceButton whack_mole_button[3];

  Inputs():
    hit(&_hit_config, PIN::HIT_BUTTON, HIGH, 0)
  {
    whack_mole_button[0].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON1, HIGH, 0);
    whack_mole_button[1].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON2, HIGH, 1);
    whack_mole_button[2].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON3, HIGH, 2);
  }

  static void handle_hit(AceButton* button, uint8_t event_type, uint8_t buttonState) {
    if(event_type != AceButton::kEventPressed) return;
    data.strip_bounce_score[data.strip_bounce_current_x] += 60;
    // data.buzzer_musician.getMelody()->restart();
    // data.buzzer_musician.stop();
    // data.buzzer_musician.play();
    // data.buzzer_musician.refresh();
    Serial.print("HIT: ");
    Serial.print(data.strip_bounce_current_x);
    Serial.print(" -> ");
    Serial.println(data.strip_bounce_score[data.strip_bounce_current_x]);
  }

  void setup() {
    // Strip
    pinMode(PIN::HIT_BUTTON, INPUT_PULLUP);
    _hit_config.setEventHandler(&handle_hit);

    // Mole
    for (auto & button : whack_mole_button) {
      pinMode(button.getPin(), INPUT_PULLUP);
    }

  }

  void loop() {
    hit.check();
    for (auto &button: whack_mole_button)
      button.check();
  }

} input;


// Helpers
namespace {


void dump() {
  data.dump();
}


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

COROUTINE(write_outputs) {
  COROUTINE_LOOP() {
    constexpr int FPS = 40;
    constexpr int delay = 1000 / FPS;
    constexpr float BRIGHTNESS = 0.30;
    FastLED.show(static_cast<uint8_t>(BRIGHTNESS*255));
    _print_fps();
    COROUTINE_DELAY(delay);
  }
}

namespace StripBounce {
struct StripBounce : Coroutine {
  void setupCoroutine() override { }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      constexpr int SPEED = 10,
        DELAY = 1000 / SPEED,
        HUE_SPEED = 1,
        SATURATION = 200;
      constexpr uint8_t SCORE_DECAY = 3;
      static int8_t direction = 1;
      static uint8_t hue = 0;

      hue += HUE_SPEED;
      auto *score = &data.strip_bounce_score[data.strip_bounce_current_x];
      output.strip[data.strip_bounce_current_x].setHSV(hue, SATURATION, *score);
      *score = *score > SCORE_DECAY ? *score - SCORE_DECAY : 0;
      switch (data.strip_bounce_current_x) {
        case 0: direction = 1; break;
        case output.STRIP_N - 1: direction = -1; break;
      }
      data.strip_bounce_current_x += direction;
      output.strip[data.strip_bounce_current_x] = CRGB::White;
      COROUTINE_DELAY(DELAY);
    }
  }
} strip_bounce;
}

namespace simple_blinker {
Led onboard_led {PIN::HEALTH_CHECK_LED};
class SimpleBlinker : Coroutine {
  int runCoroutine() override {
    COROUTINE_LOOP() {
      onboard_led.toggle();
      COROUTINE_DELAY(500);
      DEBUG("HealthCheck");
    }
  }
  
  void setupCoroutine() override {
    onboard_led.setup();
  }
} simple_blinker;
}

// #include "whack_mole.hpp"
// #include "buzzer.hpp"

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
