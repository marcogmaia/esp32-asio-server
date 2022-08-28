#pragma once

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

constexpr gpio_num_t kGpioLed = GPIO_NUM_2;

// 7 segments pins
constexpr gpio_num_t kPinSegA = GPIO_NUM_NC;
constexpr gpio_num_t kPinSegB = GPIO_NUM_NC;
constexpr gpio_num_t kPinSegC = GPIO_NUM_NC;
constexpr gpio_num_t kPinSegD = GPIO_NUM_NC;
constexpr gpio_num_t kPinSegE = GPIO_NUM_NC;
constexpr gpio_num_t kPinSegF = GPIO_NUM_NC;
constexpr gpio_num_t kPinSegG = GPIO_NUM_NC;

constexpr gpio_num_t kPinLdr            = GPIO_NUM_NC;
constexpr gpio_num_t kPinButton         = GPIO_NUM_NC;
constexpr gpio_num_t kPinLed            = GPIO_NUM_NC;
constexpr gpio_num_t kPinSensorMovement = GPIO_NUM_NC;
constexpr gpio_num_t kPinBuzzer         = GPIO_NUM_NC;

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
