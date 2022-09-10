

#include <driver/ledc.h>
#include <esp_err.h>

#include "mmrr/configs.h"
#include "mmrr/semaphore.h"

namespace mmrr::semaphore {

void BuzzerSetFrequency(int frequency) {
  // const int frequency = static_cast<int>(note);
  // ledctimer
  ledc_set_freq(kPwmSpeedMode, kPwmTimer, frequency);
}

void BuzzerTurnOff() {
  ESP_ERROR_CHECK(ledc_set_duty(kPwmSpeedMode, kPwmChannel, 0));
  ESP_ERROR_CHECK(ledc_update_duty(kPwmSpeedMode, kPwmChannel));
}

// Set duty to ~1.56%
void BuzzerTurnOn() {
  ESP_ERROR_CHECK(ledc_set_duty(kPwmSpeedMode, kPwmChannel, 1));
  ESP_ERROR_CHECK(ledc_update_duty(kPwmSpeedMode, kPwmChannel));
}

}  // namespace mmrr::semaphore
