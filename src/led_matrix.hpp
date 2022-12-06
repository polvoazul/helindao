#include <Arduino.h>
#include <FastLED.h>
#include <AceButton.h>

#include "pins.hpp"
#include "logger.hpp"
#include "coroutines.hpp"

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
    controller->setDither(0);
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
  // int Coroutine_SwapImage::setupCoroutine() {setName("swap_image");}

}
