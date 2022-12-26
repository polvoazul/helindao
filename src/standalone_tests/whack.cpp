#include <Arduino.h>
#include <AceButton.h>

#define DEBUGGING 1
// #define HELINDAO_SERIAL_BUTTON_DEBUG_SLOW 1

#include "pins.hpp"
#include "serial_button.hpp"
#include "logger.hpp"


constexpr uint8_t 
  PIN_BUTTON_DATA_IN = PIN::WHACK_MOLE_DB,
  PIN_BUTTON_CLOCK   = PIN::WHACK_MOLE_DL_CB,
  PIN_BUTTON_LOAD    = PIN::WHACK_MOLE_LB,
  PIN_LED_CLOCK      = PIN::WHACK_MOLE_CL,
  PIN_LED_DATA_OUT   = PIN::WHACK_MOLE_DL_CB;

using ace_button::AceButton;

constexpr uint WHACK_MOLE_N = 6;
SerialOutput<WHACK_MOLE_N> serial_output {.data_pin = PIN_LED_DATA_OUT, .clock_pin = PIN_LED_CLOCK};
SerialButton<WHACK_MOLE_N> button_config {.load_pin = PIN_BUTTON_LOAD, .data_pin = PIN_BUTTON_DATA_IN, .clock_pin = PIN_BUTTON_CLOCK}; 
AceButton buttons[WHACK_MOLE_N];

void handle_button(AceButton* button, uint8_t event_type, uint8_t buttonState){
  Serial << "Button " << button->getId() << " pressed with event_type: " << event_type << endl;
}

void setup2() {
  Serial.begin(115200);
  Serial.println("Alive");
  Serial.setTimeout(10);
  serial_output.setup();
  button_config.setup();

  button_config.setEventHandler(&handle_button);
  for (int i=0; i<WHACK_MOLE_N; i++)
    buttons[i].init(&button_config, /* virtual pin */ i + 100, HIGH, i);
}

void loop2() {
  delay(5);
  button_config.refresh();
  for (auto & button : buttons)
    button.check();
  if (Serial.available() != 0) {
    auto string = Serial.readString();
    Serial << string;
    for(int i=0; i<WHACK_MOLE_N; ++i)
      serial_output.set(i, LOW);
    char s = string[0];
    switch (s) {
      case '1': serial_output.set(0, HIGH); break;
      case '2': serial_output.set(1, HIGH); break;
      case '3': serial_output.set(2, HIGH); break;
      case '4': serial_output.set(3, HIGH); break;
      case '5': serial_output.set(4, HIGH); break;
      case '6': serial_output.set(5, HIGH); break;
      default: LOG("CRAP!");
    }
    LOG("Writing");
    serial_output.write();
    LOG("WRITTEN");
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("Alive");
  Serial.setTimeout(10);
  serial_output.setup();
  button_config.setup();
}

void _clear() {
  for(int i=0; i<WHACK_MOLE_N; ++i)
    serial_output.set(i, LOW);
}

void loop() {
  delay(5);
  if (Serial.available() != 0) {
    auto string = Serial.readString();
    Serial << string;
    char s = string[0];
    switch (s) {
      case '1': _clear(); serial_output.set(0, HIGH); serial_output.write(); break;
      case '2': _clear(); serial_output.set(1, HIGH); serial_output.write(); break;
      case '3': _clear(); serial_output.set(2, HIGH); serial_output.write(); break;
      case '4': _clear(); serial_output.set(3, HIGH); serial_output.write(); break;
      case '5': _clear(); serial_output.set(4, HIGH); serial_output.write(); break;
      case '6': _clear(); serial_output.set(5, HIGH); serial_output.write(); break;
      case 'a': digitalWrite(PIN::WHACK_MOLE_DL_CB, LOW); break;
      case 'A': digitalWrite(PIN::WHACK_MOLE_DL_CB, HIGH); break;
      case 'Q': button_config.refresh(); __attribute__ ((fallthrough));
      case 'p':
        Serial << ARR(button_config.last_reading) << endl;
        Serial << "State of clock led: " << digitalRead(PIN::WHACK_MOLE_CL) << endl;
        break;
      case 'q': 
        digitalWrite(PIN::WHACK_MOLE_DL_CB, LOW); digitalWrite(PIN::WHACK_MOLE_DL_CB, HIGH); 
        Serial << "Read " << digitalRead(PIN::WHACK_MOLE_DB) << endl;
        break;
      case 'W': for(int i=0; i<1000; ++i) digitalWrite(PIN::WHACK_MOLE_DL_CB, i%2); break;
      case 'E': for(int i=0; i<1000; ++i) digitalWrite(PIN::WHACK_MOLE_LB, i%2); break;
      case 'b': digitalWrite(PIN::WHACK_MOLE_CL, LOW); break;
      case 'B': digitalWrite(PIN::WHACK_MOLE_CL, HIGH); break;
      case 'c': digitalWrite(PIN::WHACK_MOLE_LB, LOW); break; // load on low
      case 'C': digitalWrite(PIN::WHACK_MOLE_LB, HIGH); break;
      case 'd': Serial << "Read " << digitalRead(PIN::WHACK_MOLE_DB) << endl; break;
      default: LOG("CRAP!");
    }
    //LOG("Writing");
    //serial_output.write();
    LOG("Ok.");
  }
}
