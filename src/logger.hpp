#pragma once

#include <Streaming.h>

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
