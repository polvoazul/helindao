#include <Arduino.h>
#include <FastLED.h>
#include <AceButton.h>
#include <Preferences.h>

#include "pins.hpp"
#include "logger.hpp"
#include "coroutines.hpp"
#include "utils.hpp"

#define HELINDAO_LED_MATRIX

namespace LedMatrix {
  #include "./matrix_images/8x8_characters.cpp"
  #include "./matrix_images/alphabet.hpp"
  const auto & BRIGHTNESS = preferences.lm_brightness;
  static constexpr int MATRIX_N = 8;
  CRGB matrix[MATRIX_N][MATRIX_N];
  CLEDController *controller;
  bool refresh_needed = true;

  struct LedMatrix : public BaseCoroutine {

  void setupCoroutine() override {
    setName("led_matrix");
    controller = &FastLED.addLeds<NEOPIXEL, PIN::MATRIX_LED_DATA>((CRGB*)matrix, MATRIX_N*MATRIX_N);
    controller->setDither(0);
  }

  int runCoroutine() override {
    // return ball_bounce();
    // return swap_image();
    return iterateLetters();
  }

  int i = 0, j = 0;

  int ball_bounce() {
    COROUTINE_LOOP() {
      for(j=0; j<8; j++)
        for(i=0; i<8; i++) {
          refresh_needed = true;
          set(i, j) = CRGB::White;
          COROUTINE_DELAY(100);
          set(i, j) = CRGB::Black;
        }
    }
  }

  const L::bit8x8 *letter;
  int iterateLetters() {
    constexpr int DELAY = 2000;
    COROUTINE_LOOP() {
      static const String word = "helio";
      for (i=0; i<word.length(); i++){
        letter = &L::get_letter(word[i]);
        for (j=0; j<(DELAY/(1000/25)); j++) { // repaint the same letter
          paint_letter(*letter, &set);
          refresh_needed = true;
          COROUTINE_DELAY(1000/25);
        }
      }
    }
  }

  int swap_image() {
    constexpr int SWAP_IMAGE_TIME_SECONDS = 15;
    COROUTINE_LOOP() {
      const int next_image = random() % N_IMAGES; // N_IMAGES = 100
      const unsigned char (&image)[8][8][3] = images8x8_characters[next_image];
      for (int y=0; y<8; ++y) {  // OPT: Copy memory in chunks
        for (int x=0; x<8; ++x) {
          set(x, y) = CRGB{image[y][x][0], image[y][x][1], image[y][x][2]};
        }
      }
      DEBUG("Image chosen: "); 
      DEBUG(next_image); 
      refresh_needed = true;
      COROUTINE_DELAY_SECONDS(SWAP_IMAGE_TIME_SECONDS);
    }
  }

  static inline CRGB & set(int x, int y) {return ((CRGB*)matrix)[XY(x, (MATRIX_N -1) - y)];}
  static uint16_t XY(uint8_t x, uint8_t y) { // from https://github.com/FastLED/FastLED/blob/master/examples/XYMatrix/XYMatrix.ino
    // We can optimize by pre-building a lookup array
    constexpr bool kMatrixSerpentineLayout = true, kMatrixVertical = false;
    constexpr int kMatrixWidth = MATRIX_N, kMatrixHeight = MATRIX_N;
    uint16_t i;
    if (kMatrixSerpentineLayout == false) {
      if (kMatrixVertical == false) {
        i = (y * kMatrixWidth) + x;
      } else {
        i = kMatrixHeight * (kMatrixWidth - (x + 1)) + y;
      }
    }
    if (kMatrixSerpentineLayout == true) {
      if (kMatrixVertical == false) {
        if (y & 0x01) {
          // Odd rows run backwards
          uint8_t reverseX = (kMatrixWidth - 1) - x;
          i = (y * kMatrixWidth) + reverseX;
        } else {
          // Even rows run forwards
          i = (y * kMatrixWidth) + x;
        }
      } else { // vertical positioning
        if (x & 0x01) {
          i = kMatrixHeight * (kMatrixWidth - (x + 1)) + y;
        } else {
          i = kMatrixHeight * (kMatrixWidth - x) - (y + 1);
        }
      }
    }
    return i;
  }

  } led_matrix;


}
