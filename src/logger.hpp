#pragma once

#include <Streaming.h>

#define LOG(x) Serial.println((x))
#define TOKEN_PASTE(x, y) x##y
#define CAT(x,y) TOKEN_PASTE(x,y)
#define UNIQUE_INT CAT(__debug_slow, __LINE__)
#define DEBUG_SLOW(x) static ulong UNIQUE_INT; if (millis() - UNIQUE_INT > 5000) {LOG(x); UNIQUE_INT = millis();}

#ifdef DEBUGGING
  #define DEBUG(x) Serial << (x) << endl
  Print & DEBUG = Serial;
#else
  #define DEBUG(x) do {} while (0)
  struct DummyStream : Print {
    inline size_t write(uint8_t) {return 1;}
    inline size_t write(const uint8_t *buffer, size_t size) {return size;}
  } __nooper;
  Print & DEBUG = __nooper;
#endif


template <typename T>
struct __StreamableArr {
  T (&array);
  __StreamableArr(T &arr): array(arr) {}

 friend Print& operator<<(Print& s, const __StreamableArr<T>& arr) {
  for (auto &el : arr.array)
    s << el;
  return s;
 }
};


#define ARR(arr) __StreamableArr<typeof(arr)>(arr)

/////////////////// UTILS /////////////////////////

constexpr uint8_t Brightness(float b) {return static_cast<uint8_t>(b*255);}
