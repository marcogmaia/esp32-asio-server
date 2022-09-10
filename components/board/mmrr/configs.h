#pragma once

#include <driver/adc.h>
#include <driver/gpio.h>
#include <driver/ledc.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

constexpr gpio_num_t kGpioLed = GPIO_NUM_2;

// constexpr int kPassword = 5;

// 7 segments pins
constexpr gpio_num_t kPinSegA = GPIO_NUM_5;
constexpr gpio_num_t kPinSegB = GPIO_NUM_18;
constexpr gpio_num_t kPinSegC = GPIO_NUM_19;
constexpr gpio_num_t kPinSegD = GPIO_NUM_21;
constexpr gpio_num_t kPinSegE = GPIO_NUM_22;
constexpr gpio_num_t kPinSegF = GPIO_NUM_14;
constexpr gpio_num_t kPinSegG = GPIO_NUM_12;

constexpr gpio_num_t kPinButton = GPIO_NUM_27;
constexpr gpio_num_t kPinBuzzer = GPIO_NUM_23;
// constexpr gpio_num_t kPinLdr            = GPIO_NUM_NC; // GPIO_34 - ADC
constexpr gpio_num_t kPinLed            = GPIO_NUM_13;
constexpr gpio_num_t kPinSensorMovement = GPIO_NUM_NC;

// Semaphores led pins.
constexpr gpio_num_t kPinLedGreen  = GPIO_NUM_13;
constexpr gpio_num_t kPinLedYellow = GPIO_NUM_32;
constexpr gpio_num_t kPinLedRed    = GPIO_NUM_33;

// Wifi configs.
constexpr auto kWifiSsid     = "CINGUESTS";
constexpr auto kWifiPass     = "acessocin";
constexpr auto kMaximumRetry = 2;

// Client configs.
constexpr const char* kPort = "54321";

// Adc configs.
// GPIO32 ADC1_CH4
// GPIO33 ADC1_CH5
// GPIO34 ADC1_CH6
// GPIO35 ADC1_CH7
// GPIO36 ADC1_CH0
// GPIO37 ADC1_CH1
// GPIO38 ADC1_CH2
// GPIO39 ADC1_CH3
constexpr adc_unit_t kAdcUnit         = ADC_UNIT_1;
constexpr adc_channel_t kAdcChannel   = ADC_CHANNEL_6;  // GPIO34 if ADC1, GPIO14 if ADC2
constexpr adc_bits_width_t kAdcWidth  = ADC_WIDTH_BIT_10;
constexpr adc_atten_t kAdcAttenuation = ADC_ATTEN_DB_0;

// PWM configs.
constexpr gpio_num_t kPinPwm      = kPinBuzzer;
constexpr auto kPwmChannel        = ledc_channel_t::LEDC_CHANNEL_0;
constexpr auto kPwmSpeedMode      = ledc_mode_t::LEDC_HIGH_SPEED_MODE;
constexpr auto kPwmDutyResolution = ledc_timer_bit_t::LEDC_TIMER_10_BIT;
constexpr auto kPwmTimer          = ledc_timer_t::LEDC_TIMER_0;
constexpr int kPwmFrequency       = 20'000;
constexpr int kPwmInitialDuty     = 0;  // 0% of 6 bit.

template <typename GpioArray>
constexpr uint64_t ComputeBitMask(GpioArray gpio_array) {
  uint64_t bitmask = 0;
  for (auto& gpio : gpio_array) {
    if (gpio >= 0) {
      bitmask |= BIT64(gpio);
    }
  }
  return bitmask;
}

inline void ConfigGpio(uint64_t gpio_bitmask, gpio_mode_t mode) {
  const gpio_config_t config{.pin_bit_mask = gpio_bitmask,
                             .mode         = mode,
                             .pull_up_en   = gpio_pullup_t::GPIO_PULLUP_DISABLE,
                             .pull_down_en = gpio_pulldown_t::GPIO_PULLDOWN_DISABLE,
                             .intr_type    = gpio_int_type_t::GPIO_INTR_DISABLE};
  gpio_config(&config);
}

inline void BlinkFunctionTask(void* ignore) {
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    gpio_pad_select_gpio(kGpioLed);
    gpio_set_direction(kGpioLed, gpio_mode_t::GPIO_MODE_OUTPUT);
    gpio_set_level(kGpioLed, 1);
  }
  gpio_set_level(kGpioLed, 1);
  vTaskDelay(pdMS_TO_TICKS(50));
  gpio_set_level(kGpioLed, 0);
  vTaskDelete(nullptr);
}

inline void Blink() {
  xTaskCreatePinnedToCore(
      BlinkFunctionTask, "blink", configMINIMAL_STACK_SIZE, nullptr, 5, nullptr, APP_CPU_NUM);
}

namespace mmrr {

void Init();
void InitSemaphore();

}  // namespace mmrr