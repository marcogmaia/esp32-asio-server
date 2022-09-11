
#include <algorithm>
#include <array>
#include <cmath>

#include <driver/gpio.h>
#include <driver/ledc.h>

#include "impl/init.h"
#include "impl/ldr.h"
#include "mmrr/configs.h"

namespace mmrr::semaphore {

namespace detail {

void InitGpio() {
  constexpr std::array<gpio_num_t, 4> semaphore_pins{kPinLedRed, kPinLedYellow, kPinLedGreen};
  constexpr auto bitmask = ComputeBitMask(semaphore_pins);
  ConfigGpio(bitmask, gpio_mode_t::GPIO_MODE_OUTPUT);
}

void SetDuty(uint8_t duty) {
  ledc_set_duty(kPwmSpeedMode, kPwmChannel, duty);
  ledc_update_duty(kPwmSpeedMode, kPwmChannel);
}

void InitPwm() {
  // Prepare and then apply the LEDC PWM timer configuration
  ledc_timer_config_t ledc_timer = {.speed_mode      = kPwmSpeedMode,
                                    .duty_resolution = kPwmDutyResolution,
                                    .timer_num       = kPwmTimer,
                                    .freq_hz         = kPwmFrequency,
                                    .clk_cfg         = LEDC_USE_APB_CLK};
  ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

  // Prepare and then apply the LEDC PWM channel configuration
  ledc_channel_config_t ledc_channel = {.gpio_num   = kPinPwm,
                                        .speed_mode = kPwmSpeedMode,
                                        .channel    = kPwmChannel,
                                        .intr_type  = LEDC_INTR_DISABLE,
                                        .timer_sel  = kPwmTimer,
                                        .duty       = 0,
                                        .hpoint     = 0,
                                        .flags      = {.output_invert = 0}};
  ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void InitializeSegments() {
  constexpr std::array<gpio_num_t, 7> segment_pins{
      kPinSegA,
      kPinSegB,
      kPinSegC,
      kPinSegD,
      kPinSegE,
      kPinSegF,
      kPinSegG,
  };
  constexpr auto segs_bitmask = ComputeBitMask(segment_pins);
  ConfigGpio(segs_bitmask, gpio_mode_t::GPIO_MODE_OUTPUT);
  for (auto& pin : segment_pins) {
    gpio_set_level(pin, 0);
  }
}

}  // namespace detail

void Init() {
  detail::InitGpio();
  detail::InitPwm();
  InitLdr();
  detail::InitializeSegments();
  detail::InitBlynk();
}

}  // namespace mmrr::semaphore
