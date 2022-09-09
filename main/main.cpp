#include <exception>
#include <iostream>

#include <driver/gpio.h>
#include <esp_log.h>
#include <fmt/format.h>
#include <nvs_flash.h>
#include <asio.hpp>

#include "adc.h"
#include "alarm/alarm.h"
#include "board_configs.h"
#include "client.h"
#include "queue.h"
#include "uart.h"
#include "wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

constexpr auto kTag = "Main";

void InitNvs() {
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);
}

void app_main(void) {
  // Initialize NVS
  InitNvs();

  ESP_LOGI(kTag, "ESP_WIFI_MODE_STA");
  mmrr::wifi::Init();

  mmrr::queue::Init();
  mmrr::uart::Init();
  mmrr::adc::Init();
  mmrr::client::Init();
  mmrr::alarm::Init();
}

#ifdef __cplusplus
}
#endif