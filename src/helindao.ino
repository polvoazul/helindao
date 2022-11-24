#include <Arduino.h>
#include <FastLED.h>
#include <AceRoutine.h>
#include <AceButton.h>
#include <Melody.h>
#include <Musician.h>


using namespace ace_button;
using namespace ace_routine;

#define LOG(x) Serial.println((x));


using ace_routine::CoroutineScheduler;

constexpr uint8_t
  PIN_BUTTON = 2,
  PIN_BUZZER = 3,
  PIN_LED_STRIP_DATA = 4,
  LEDC_CHANNEL_BUZZER = 0;

// GLOBALS
struct Outputs {
  static constexpr int STRIP_N = 30;
  static constexpr int MATRIX_N = 8;
  CRGB matrix[MATRIX_N][MATRIX_N];
  CRGB strip[STRIP_N];

  void setup() {
    // Setup Leds
    constexpr EOrder RGB_ORDER = EOrder::RGB;
    // FastLED.addLeds<LED_MODEL>((CRGB*)matrix, MATRIX_N * MATRIX_N);
    FastLED.addLeds<NEOPIXEL, PIN_LED_STRIP_DATA>((CRGB*)strip, STRIP_N);
  }
} output;

// DATA
struct Data {
  uint8_t strip_bounce_score[output.STRIP_N] = {};
  uint8_t strip_bounce_current_x = 0;
  Musician buzzer_musician = Musician( PIN_BUZZER, LEDC_CHANNEL_BUZZER );

  void setup() {}
  void dump() {
    for (int i : strip_bounce_score) {
      Serial.print("Score"); Serial.println(i);
    }
  }

} data;



// INPUTS

class Inputs {
public:
  AceButton hit{ PIN_BUTTON };

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

  void setup() {
    pinMode(PIN_BUTTON, INPUT_PULLUP);
    hit.getButtonConfig()->setEventHandler(&handle_hit);
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
  constexpr int FPS = 40;
  constexpr int delay = 1000 / FPS;
  COROUTINE_LOOP() {
    FastLED.show();
    _print_fps();
    COROUTINE_DELAY(delay);
  }
}


COROUTINE(strip_bounce) {
  constexpr int SPEED = 20,
    DELAY = 1000 / SPEED,
    HUE_SPEED = 1,
    SATURATION = 200;
  constexpr uint8_t SCORE_DECAY = 3;
  static int8_t direction = 1;
  static uint8_t hue = 0;
  COROUTINE_LOOP() {
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

const char melody_simple[] = "(cge+)*";

class BuzzerTone: public Coroutine {
  public:
  
  Melody melody;

  void setup() {
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

  buzzer_tone.setup();



  CoroutineScheduler::setup();
  LogBinProfiler::createProfilers();
}

void loop() {
  CoroutineScheduler::loop();
  if (Serial.available() > 0) {  
    while (Serial.available()) {Serial.read();} 
    dump();
  }
}
