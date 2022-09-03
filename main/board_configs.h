#pragma once

#include "driver/adc.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

constexpr gpio_num_t kPinLdr            = GPIO_NUM_NC;
constexpr gpio_num_t kPinButton         = GPIO_NUM_NC;
constexpr gpio_num_t kPinLed            = GPIO_NUM_NC;
constexpr gpio_num_t kPinSensorMovement = GPIO_NUM_NC;
constexpr gpio_num_t kPinBuzzer         = GPIO_NUM_NC;

// Wifi configs.
// constexpr auto kWifiSsid     = "CINGUESTS";
// constexpr auto kWifiPass     = "acessocin";
constexpr auto kWifiSsid     = "maia";
constexpr auto kWifiPass     = "marco3445";
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
