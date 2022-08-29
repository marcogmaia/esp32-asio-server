#include <exception>
#include <iostream>

#include <driver/gpio.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <asio.hpp>

#include "alarm/alarm.h"
#include "board_configs.h"
#include "server.h"
#include "uart.h"
#include "wifi_server.h"

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
  WifiInitStation();

  // mmrr::alarm::Init();
  mmrr::uart::Init();

  try {
    asio::io_context io_context;
    uint16_t port = 54321;
    Server server(io_context, port);
    io_context.run();
  } catch (std::exception& e) {
    ESP_LOGE(kTag, "Exception: %s", e.what());
  }
}

#ifdef __cplusplus
}
#endif