#include <algorithm>
#include <cmath>

#include <driver/ledc.h>
#include <esp_err.h>

#include "mmrr/configs.h"
#include "mmrr/semaphore.h"

namespace mmrr::semaphore {

namespace {

void SetDuty(uint8_t duty) {
  ledc_set_duty(kPwmSpeedMode, kPwmChannel, duty);
  ledc_update_duty(kPwmSpeedMode, kPwmChannel);
}

void SetDutyPercentual(int percentual) {
  constexpr uint32_t kMaxDuty = ((1 << kPwmDutyResolution) - 1);
  constexpr double kPercent   = kMaxDuty / 100.0;
  const int duty              = std::clamp(
      static_cast<int>(std::round(percentual * kPercent)), 0, static_cast<int>(kMaxDuty));
  SetDuty(duty);
}

void BuzzerSetFrequency(int frequency) {
  ledc_set_freq(kPwmSpeedMode, kPwmTimer, frequency);
}

}  // namespace

void BuzzerTurnOff() {
  SetDutyPercentual(0);
}

// Set duty to ~1.56%
void BuzzerTurnOn() {
  BuzzerSetFrequency(GetBuzzerFrequency());
  SetDutyPercentual(20);
}

}  // namespace mmrr::semaphore
