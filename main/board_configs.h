#pragma once

#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

constexpr auto kGpioLed = GPIO_NUM_2;

inline void BlinkFunctionTask(void* ignore) {
  static bool initialized = false;
  if (!initialized) {
    initialized = true;
    gpio_pad_select_gpio(kGpioLed);
    gpio_set_direction(kGpioLed, gpio_mode_t::GPIO_MODE_OUTPUT);
    gpio_set_level(kGpioLed, 1);
  }
  gpio_set_level(kGpioLed, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(kGpioLed, 0);
  vTaskDelete(nullptr);
}

inline void Blink() {
  xTaskCreatePinnedToCore(
      BlinkFunctionTask, "blink", configMINIMAL_STACK_SIZE, nullptr, 5, nullptr, APP_CPU_NUM);
}
