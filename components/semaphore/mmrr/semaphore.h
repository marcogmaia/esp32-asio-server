#pragma once

namespace mmrr::semaphore {

struct SemaphoreTiming {
  int red    = 0;
  int yellow = 0;
  int green  = 0;
};

enum class Notes {
kC3 = 128,
kE3 = 160,
kG3 = 200,
kC5 = 512,
kE5 = 640,
kA5 = 800,
};

SemaphoreTiming GetSemaphoreTiming();

void BuzzerSetFrequency(int frequency);
void BuzzerTurnOff();
void BuzzerTurnOn();
bool IsBuzzerOn();

void Init();

}  // namespace mmrr::semaphore
