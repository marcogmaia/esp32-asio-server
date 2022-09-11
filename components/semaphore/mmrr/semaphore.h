#pragma once

namespace mmrr::semaphore {

struct SemaphoreTiming {
  int red    = 0;
  int yellow = 0;
  int green  = 0;
};

SemaphoreTiming GetSemaphoreTiming();

void BuzzerSetFrequency(int frequency);
void BuzzerTurnOff();
void BuzzerTurnOn();
bool IsBuzzerOn();
bool IsBuzzerStateChanged();
int GetBuzzerFrequency();

/// @brief Count a car.
/// @return The actual number of counted cars.
int AddCarCounter();

void Init();

}  // namespace mmrr::semaphore
