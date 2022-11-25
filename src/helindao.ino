#include <Arduino.h>
#include <FastLED.h>
#include <AceRoutine.h>
#include <AceButton.h>
#include <Melody.h>
#include <Musician.h>
#include "led.cpp"

#include <vector>

using namespace ace_button;
using namespace ace_routine;

#define LOG(x) Serial.println((x));


using ace_routine::CoroutineScheduler;



namespace PIN { enum ENUM : uint8_t {
  BUTTON = 2,
  LED_STRIP_DATA = 4,
  BUZZER = 5,

  WHACK_MOLE_BUTTON1 = 21,
  WHACK_MOLE_BUTTON2 = 22,
  WHACK_MOLE_BUTTON3 = 23,
  WHACK_MOLE_LED1 = 25,
  WHACK_MOLE_LED2 = 26,
  WHACK_MOLE_LED3 = 27,

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
    // constexpr EOrder RGB_ORDER = EOrder::RGB;
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


  void setup() = delete; // data needs to be dumb

  void dump() {
    for (int i : strip_bounce_score) {
      Serial.print("Score"); Serial.println(i);
    }
  }

} data;



// INPUTS

struct Inputs { // convert to namespace and globals?
  ButtonConfig _hit_config, _whack_mole_config;
  AceButton hit;
  AceButton whack_mole_button[3];

  Inputs():
    hit(&_hit_config, PIN::BUTTON, HIGH, 0)
  {
    whack_mole_button[0].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON1, HIGH, 0);
    whack_mole_button[1].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON2, HIGH, 1);
    whack_mole_button[2].init(&_whack_mole_config, PIN::WHACK_MOLE_BUTTON3, HIGH, 2);
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

  void setup() {
    // Strip
    pinMode(PIN::BUTTON, INPUT_PULLUP);
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

namespace WhackMole {
bool whack_mole_present[WHACK_MOLE_N];
int whack_mole_score;
enum State {
  ON,
  WINNING
} state = ON;
constexpr int WINNING_BLINKS_N = 3;

struct WhackMole : public Coroutine {
  static constexpr int INITIAL_MOLES = 1;

  static void handle_whack_button(AceButton* button, uint8_t event_type, uint8_t buttonState){
    if(state == WINNING) return;
    if(event_type != AceButton::kEventPressed) return;
    int whacked_mole = button->getId();
    if(!whack_mole_present[whacked_mole]) return;
    // whacked!
    set_mole(whacked_mole, false);
    int next = random() % WHACK_MOLE_N - 1;
    if(next == whacked_mole) next = WHACK_MOLE_N - 1;
    set_mole(next);
    whack_mole_score += 1;
    if(whack_mole_score > 10)
      state = WINNING;
  }

  static void set_mole(int i, bool on=true) {
    assert(i < WHACK_MOLE_N);
    whack_mole_present[i] = on;
    output.whack_mole_leds[i].set(on); // move to output loop?
  }


  void setupCoroutine() override {
    for(bool & mole : whack_mole_present)
      mole = false;
    whack_mole_score = 0;
    input._whack_mole_config.setEventHandler(&handle_whack_button);
    for(int i=0; i<INITIAL_MOLES; i++) {
      int mole = random() % WHACK_MOLE_N;
      set_mole(mole);
    }
  }

  void set_all_whack_leds(bool on = true) {
    for(auto & l : output.whack_mole_leds)
      l.set(on); // move to output loop?
  }

  int winning_blinks = 0;

  int runCoroutine() override {
    COROUTINE_LOOP() {
      if(state == WINNING) {
        for(winning_blinks=0; winning_blinks < WINNING_BLINKS_N; winning_blinks++) {
          set_all_whack_leds(true);
          COROUTINE_DELAY_SECONDS(1);
          set_all_whack_leds(false);
          COROUTINE_DELAY_SECONDS(1);
        }
        state = ON;
        whack_mole_score = 0;
      }
      COROUTINE_DELAY(100); // change to pause / unpause
    }
  }
} whack_mole;
}
// Helpers

void dump() {
  data.dump();
}

// Arduino CODE

void setup() {
  Serial.begin(9600);
  Serial.println("Alive");
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
