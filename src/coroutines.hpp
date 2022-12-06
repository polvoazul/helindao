#pragma once
#include <AceRoutine.h>

struct BaseCoroutine : ::ace_routine::Coroutine {
};

#undef COROUTINE1
#define COROUTINE1(name) \
struct Coroutine_##name : BaseCoroutine { \
  Coroutine_##name(); \
  void setupCoroutine() override {setName(#name);} \
  int runCoroutine() override; \
} name; \
Coroutine_##name :: Coroutine_##name() { \
} \
int Coroutine_##name :: runCoroutine()
