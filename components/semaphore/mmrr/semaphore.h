#pragma once

namespace mmrr::semaphore {

struct SemaphoreTiming {
  int red    = 5000;
  int yellow = 5000;
  int green  = 5000;
};

SemaphoreTiming GetSemaphoreTiming();

// void BuzzerSetFrequency(int frequency);
void BuzzerTurnOff();
void BuzzerTurnOn();
bool IsBuzzerOn();
bool IsBuzzerStateChanged();
int GetBuzzerFrequency();

/// @brief Count a car.
/// @return The actual number of counted cars.
int AddCarCounter();

void Init();
void Fsm();

}  // namespace mmrr::semaphore
