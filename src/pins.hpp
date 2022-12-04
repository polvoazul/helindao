#ifndef HELINDAO_PINS
#define HELINDAO_PINS

#include <Arduino.h>

// file://./../utils/pinout.png https://docs.espressif.com/projects/esp-idf/en/latest/esp32/_images/esp32-devkitC-v4-pinout.png

namespace PIN {
uint8_t constexpr

/*
| VARNAME                     = GPIO   |   Input   | Output |                                    Notes                                     |
|-------------------------------------:|:---------:|:------:|------------------------------------------------------------------------------|
|**  _                        =  0  ,/*| pulled up |   OK   |outputs PWM signal at boot, must be LOW to enter flashing mode                |
|**  _                        =  1  ,/*|  TX pin   |   OK   |debug output at boot                                                          |
|**  _                        =  2  ,/*|   ~OK     |  ~OK   |connected to on-board LED, must be left floating or LOW to enter flashing mode|
|**  _                        =  3  ,/*|    OK     | RX pin |HIGH at boot                                                                  |
|*/  LED_STRIP_DATA           =  4  ,/*|    OK     |   OK   |                                                                              |
|**  _                        =  5  ,/*|   ~OK     |  ~OK   |outputs PWM signal at boot, strapping pin                                     |
|**  _                        =  6  ,/*|     x     |   x    |connected to the integrated SPI flash                                         |
|**  _                        =  7  ,/*|     x     |   x    |connected to the integrated SPI flash                                         |
|**  _                        =  8  ,/*|     x     |   x    |connected to the integrated SPI flash                                         |
|**  _                        =  9  ,/*|     x     |   x    |connected to the integrated SPI flash                                         |
|**  _                        = 10  ,/*|     x     |   x    |connected to the integrated SPI flash                                         |
|**  _                        = 11  ,/*|     x     |   x    |connected to the integrated SPI flash                                         |
|**  _                        = 12  ,/*|   ~OK     |  ~OK   |boot fails if pulled high, strapping pin                                      |
|*/  MATRIX_LED_DATA          = 13  ,/*|    OK     |   OK   |                                                                              |
|*/  BUZZER                   = 14  ,/*|   ~OK     |   OK   |outputs PWM signal at boot                                                    |
|*/  HEALTH_CHECK_LED         = 15  ,/*|   ~OK     |   OK   |outputs PWM signal at boot, strapping pin                                     |
|*/  WHACK_MOLE_BUTTON1       = 16  ,/*|    OK     |   OK   |                                                                              |
|*/  WHACK_MOLE_BUTTON2       = 17  ,/*|    OK     |   OK   |                                                                              |
|*/  WHACK_MOLE_BUTTON3       = 18  ,/*|    OK     |   OK   |                                                                              |
|*/  WHACK_MOLE_LED1          = 19  ,/*|    OK     |   OK   |                                                                              |
|*/  WHACK_MOLE_LED2          = 21  ,/*|    OK     |   OK   |                                                                              |
|*/  WHACK_MOLE_LED3          = 22  ,/*|    OK     |   OK   |                                                                              |
|**  _                        = 23  ,/*|    OK     |   OK   |                                                                              |
|**  _                        = 25  ,/*|    OK     |   OK   |DAC output 0                                                                  |
|**  _                        = 26  ,/*|    OK     |   OK   |DAC output 1                                                                  |
|**  _                        = 27  ,/*|    OK     |   OK   |                                                                              |
|**  _                        = 32  ,/*|    OK     |   OK   |                                                                              |
|**  _                        = 33  ,/*|    OK     |   OK   |                                                                              |
|*/  HIT_BUTTON               = 34  ,/*|    OK     |        |input only     no pullup                                                      |
|**  _                        = 35  ,/*|    OK     |        |input only     no pullup                                                      |
|**  _                        = 36  ,/*|    OK     |        |input only     no pullup                                                      |
|**  _                        = 39  ,/*|    OK     |        |input only     no pullup                                                      |
*/
  __END__ = 99
;
}

#endif
