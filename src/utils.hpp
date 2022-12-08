#pragma once
#include <String>
#include <Preferences.h>
#include <Streaming.h>

uint8_t Brightness(float b) {return static_cast<uint8_t>(b*255);}


struct Pref : Preferences {
  // Define prefs here, with defaults
  float lm_brightness = 0.15,
      sb_brightness = 0.2;
  int sb_speed = 10;

  void setup() {
    #define LOAD(k, t) k = get##t(#k, k)
    begin("helindao");
    LOAD(lm_brightness, Float);
    LOAD(sb_brightness, Float);
    LOAD(sb_speed, Int);
    #undef LOAD
  }

  void write(String key_and_value) {
    key_and_value.replace(" ", "");
    if(key_and_value.indexOf('=') == -1) { Serial << "Missing '=', not a valid pref setter" << endl; return;}
    String key = key_and_value.substring(0, key_and_value.indexOf('='));
    String value_s = key_and_value.substring(key_and_value.indexOf('=') + 1);

    #define Float float
    #define Int int
    #define SET(k, t) if(key == #k) { \
          t value = value_s.to##t(); \
          put##t(key.c_str(), value);  \
          Serial << "Set !" << key << " = " << value << endl; \
           restart(); \
          } else
    SET(lm_brightness, Float)
    SET(sb_brightness, Float)
    SET(sb_speed, Int)
    #undef Float
    #undef Int
    #undef SET

    // when all else fails
    Serial << "FAILED! Preference '" << key << "' unknown." << endl;
  }
  void restart() {
    end();
    Serial << "Restarting to load new prefs" << endl;
    ESP.restart();
  }
} preferences;
