#include <Arduino.h>
#include <FastLED.h>
#include <AceRoutine.h>
#include <AceButton.h>
#include <Melody.h>
#include <Musician.h>
#include "led.cc"

#include <vector>

using namespace ace_button;
using namespace ace_routine;

#define LOG(x) Serial.println((x));


using ace_routine::CoroutineScheduler;

namespace PIN { enum ENUM : uint8_t {
  BUTTON = 2,
  LED_STRIP_DATA = 4,
  BUZZER = 5,

  WHACK_MOLE_BUTTON1 = 11,
  WHACK_MOLE_BUTTON2 = 12,
  WHACK_MOLE_BUTTON3 = 13,
  WHACK_MOLE_LED1 = 16,
  WHACK_MOLE_LED2 = 17,
  WHACK_MOLE_LED3 = 18,

  __END__ = 99
};}
  
constexpr uint8_t
  LEDC_CHANNEL_BUZZER = 0,
  WHACK_MOLE_N = 3;

// OUTPUTS
struct Outputs {
  static constexpr int STRIP_N = 30;
  static constexpr int MATRIX_N = 8;
  CRGB matrix[MATRIX_N][MATRIX_N];
  CRGB strip[STRIP_N];
  Led whack_mole_leds[3] = {PIN::WHACK_MOLE_LED1, PIN::WHACK_MOLE_LED2, PIN::WHACK_MOLE_LED3};

  void setup() {
    // Setup Leds array
    constexpr EOrder RGB_ORDER = EOrder::RGB;
    // FastLED.addLeds<LED_MODEL>((CRGB*)matrix, MATRIX_N * MATRIX_N);
    FastLED.addLeds<NEOPIXEL, PIN::LED_STRIP_DATA>((CRGB*)strip, STRIP_N);

    for(Led & led : whack_mole_leds) led.setup();
  }
} output;

// DATA
struct Data {
  uint8_t strip_bounce_score[output.STRIP_N] = {};
  uint8_t strip_bounce_current_x = 0;
  Musician buzzer_musician = Musician( PIN::BUZZER, LEDC_CHANNEL_BUZZER );
  bool whack_mole_present[WHACK_MOLE_N];
  int whack_mole_score = 0;

  void setup() {}
  void dump() {
    for (int i : strip_bounce_score) {
      Serial.print("Score"); Serial.println(i);
    }
  }

} data;



// INPUTS

struct Inputs { // convert to namespace and globals?
  AceButton hit;
  AceButton whack_mole_button[3];

  Inputs():
    hit(PIN::BUTTON)
  {
    whack_mole_button[0].init(PIN::WHACK_MOLE_BUTTON1, HIGH, 0);
    whack_mole_button[1].init(PIN::WHACK_MOLE_BUTTON2, HIGH, 1);
    whack_mole_button[2].init(PIN::WHACK_MOLE_BUTTON3, HIGH, 2);
  }

  static void handle_hit(AceButton* button, uint8_t event_type, uint8_t buttonState) {
    if(event_type != AceButton::kEventPressed) return;
    data.strip_bounce_score[data.strip_bounce_current_x] += 60;
    data.buzzer_musician.getMelody()->restart();
    data.buzzer_musician.stop();
    data.buzzer_musician.play();
    data.buzzer_musician.refresh();
    Serial.print("HIT: ");
    Serial.print(data.strip_bounce_current_x);
    Serial.print(" -> ");
    Serial.println(data.strip_bounce_score[data.strip_bounce_current_x]);
  }

  static void handle_whack_button(AceButton* button, uint8_t event_type, uint8_t buttonState){
    if(event_type != AceButton::kEventPressed) return;
    int whacked_mole = button->getId();
    if(!data.whack_mole_present[whacked_mole]) return;
    // whacked!
    data.whack_mole_present[whacked_mole] = false;
    output.whack_mole_leds[whacked_mole].off(); // move to output loop?
    int next = random() % WHACK_MOLE_N - 1;
    if(next == whacked_mole) next = WHACK_MOLE_N - 1;
    data.whack_mole_present[next] = true;
    output.whack_mole_leds[whacked_mole].on(); // move to output loop?
    data.whack_mole_score += 1;
  }


  void setup() {
    pinMode(PIN::BUTTON, INPUT_PULLUP);
    hit.getButtonConfig()->setEventHandler(&handle_hit);

    for (auto & button : whack_mole_button) {
      pinMode(button.getPin(), INPUT_PULLUP);
      button.getButtonConfig()->setEventHandler(&handle_whack_button);
    }
  }

  void loop() {
    hit.check();
    for (auto &button: whack_mole_button)
      button.check();
  }

} input;




// COROUTINES

COROUTINE(read_inputs) {
  COROUTINE_LOOP() {
    input.loop();
    COROUTINE_DELAY(5);
  }
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

COROUTINE(write_outputs) {
  COROUTINE_LOOP() {
    constexpr int FPS = 40;
    constexpr int delay = 1000 / FPS;
    FastLED.show();
    _print_fps();
    COROUTINE_DELAY(delay);
  }
}


COROUTINE(strip_bounce) {
  COROUTINE_LOOP() {
    constexpr int SPEED = 20,
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

// const char melody_s[] = "cde | f ff cd c | d dd cgf | e ee cde | f ff";
// const char melody_s[] = "cde | f ff cd c | d dd cgf | e ee cde | a"; // Crazy bug hang on long melody

//Valid choice kind-of sound
// Melody melody_valid(" (cgc*)**---");

//Invalid choice kind-of sound
Melody melody_invalid(" (cg_)__");

static const char melody_simple[] = "(cge+)*";
struct BuzzerTone: public Coroutine {
  
  Melody melody;

  void setupCoroutine() override {
    melody.setScore(melody_simple);
    melody.setTempo(200);
    data.buzzer_musician.setMelody(&melody_invalid);
    data.buzzer_musician.setBreath(90);
    data.buzzer_musician.play();
    data.buzzer_musician.refresh();
  }

  int runCoroutine() override {
    COROUTINE_LOOP() {
      if (data.buzzer_musician.isPlaying()) {
        data.buzzer_musician.refresh();
      }
      COROUTINE_DELAY(3);
    }
  }
} buzzer_tone;
// Helpers

void dump() {
  data.dump();
}

// Arduino CODE

void setup() {
  Serial.begin(9600);
  Serial.println("Alive");
  data.setup();
  output.setup();
  input.setup();


  CoroutineScheduler::setup();
  CoroutineScheduler::setupCoroutines();
  LogBinProfiler::createProfilers();
}

void loop() {
  CoroutineScheduler::loop();
  if (Serial.available() > 0) {  
    while (Serial.available()) {Serial.read();} 
    dump();
  }
}
