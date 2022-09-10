#include <exception>
#include <iostream>

#include <driver/gpio.h>
#include <esp_log.h>
#include <fmt/format.h>
#include <nvs_flash.h>

#include "adc.h"
#include "client.h"
#include "mmrr/configs.h"
#include "mmrr/semaphore.h"
#include "queue.h"
#include "uart.h"

constexpr auto kTag = "Main";

void InitNvs() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

extern "C" void app_main(void) {
  InitNvs();
  ESP_LOGI(kTag, "ESP_WIFI_MODE_STA");
  mmrr::semaphore::Init();
  mmrr::queue::Init();
  mmrr::adc::Init();

  using namespace mmrr::semaphore;

  while (true) {
    if (IsBuzzerStateChanged()) {
      if (IsBuzzerOn()) {
        BuzzerTurnOn();
        BuzzerSetFrequency(GetBuzzerFrequency());
      } else {
        BuzzerTurnOff();
      }
    }
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
