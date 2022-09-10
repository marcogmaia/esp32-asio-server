
#include <array>

#include <driver/gpio.h>
#include <driver/ledc.h>

#include "impl/init.h"
#include "mmrr/configs.h"

namespace mmrr::semaphore {

namespace detail {

void InitGpio() {
  constexpr std::array<gpio_num_t, 4> semaphore_pins{
      kPinLedRed, kPinLedYellow, kPinLedGreen, kPinBuzzer};
  constexpr auto bitmask = ComputeBitMask(semaphore_pins);
  ConfigGpio(bitmask, gpio_mode_t::GPIO_MODE_OUTPUT);
}

void InitPwm() {
  // Prepare and then apply the LEDC PWM timer configuration
  ledc_timer_config_t ledc_timer = {.speed_mode      = kPwmSpeedMode,
                                    .duty_resolution = kPwmDutyResolution,
                                    .timer_num       = kPwmTimer,
                                    .freq_hz = kPwmFrequency,  // Set output frequency at 5 kHz
                                    .clk_cfg = LEDC_AUTO_CLK};
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

}  // namespace detail

void Init() {
  detail::InitGpio();
  detail::InitPwm();
  detail::InitBlynk();
}

}  // namespace mmrr::semaphore
