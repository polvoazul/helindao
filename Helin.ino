#include "FastLED.h"
#include <AceRoutine.h>
#include <AceButton.h>
#include <Musician.h>

using namespace ace_button;


using ace_routine::CoroutineScheduler;

constexpr uint8_t
  PIN_LED_STRIP_DATA = 4,
  PIN_BUZZER = 3;

// GLOBALS
struct Outputs {
  static constexpr int STRIP_N = 30;
  static constexpr int MATRIX_N = 8;
  // CRGB matrix[MATRIX_N][MATRIX_N];
  CRGB strip[STRIP_N];

  void setup() {
    // Setup Leds
    constexpr EOrder RGB_ORDER = 1;
#define LED_MODEL NEOPIXEL, PIN_LED_STRIP_DATA
    // FastLED.addLeds<LED_MODEL>((CRGB*)matrix, MATRIX_N * MATRIX_N);
    FastLED.addLeds<LED_MODEL>((CRGB*)strip, STRIP_N);
#undef LED_MODEL
  }
} output;

// DATA
struct Data {
  uint8_t strip_bounce_score[output.STRIP_N] = {};
  uint8_t strip_bounce_current_x = 0;
  Musician buzzer_musician{ PIN_BUZZER };

  void setup() {}

} data;



// INPUTS

class Inputs {
public:
  static constexpr int HIT_PIN = 10;
  AceButton hit{ HIT_PIN };

  static void handle_hit() {
    data.strip_bounce_score[data.strip_bounce_current_x] += 1;
  }

  void setup() {
    pinMode(HIT_PIN, INPUT_PULLUP);
    hit.getButtonConfig()->setEventHandler(handle_hit);
  }

  void read() {
    hit.check();
  }

} input;


// COROUTINES

COROUTINE(read_inputs) {
  COROUTINE_LOOP() {
    input.read();
    COROUTINE_DELAY(5);
  }
}

COROUTINE(write_outputs) {
  constexpr int FPS = 30;
  constexpr int delay = 1000 / FPS;
  COROUTINE_LOOP() {
    FastLED.show();
    COROUTINE_DELAY(delay);
  }
}


COROUTINE(strip_bounce) {
  constexpr int SPEED = 100;
  constexpr int DELAY = 1000 / SPEED;
  constexpr int SATURATION = 200;
  static int8_t direction = 1;
  static uint8_t hue = 0;
  COROUTINE_LOOP() {
    hue += 5;
    output.strip[data.strip_bounce_current_x].setHSV(
      hue, SATURATION, data.strip_bounce_score[data.strip_bounce_current_x]);
    switch (data.strip_bounce_current_x) {
      case 0: direction = 1; break;
      case output.STRIP_N - 1: direction = -1; break;
    }
    data.strip_bounce_current_x += direction;
    output.strip[data.strip_bounce_current_x] = CRGB::White;
    COROUTINE_DELAY(DELAY);
  }
}

COROUTINE(buzzer_tone) {
  constexpr auto melody = "cde | f ff cd c | d dd cgf | e ee cde | f ff";
  static struct A {int b = 2;} a;
  static auto fred = []() {return 1;};
  COROUTINE_LOOP() {
    data.buzzer_musician.refresh();
  }
}



// Arduino CODE

void setup() {
  data.setup();
  output.setup();
  input.setup();

  CoroutineScheduler::setup();
}

void loop() {
  CoroutineScheduler::loop();
}