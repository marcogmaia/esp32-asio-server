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
  mmrr::queue::Init();
  mmrr::uart::Init();
  mmrr::adc::Init();
  mmrr::client::Init();

  // try 5
  //   asio::io_context io_context;
  //   uint16_t port = 54321;
  //   Server server(io_context, port);
  //   io_context.run();
  // } catch (std::exception& e) {
  //   ESP_LOGE(kTag, "Exception: %s", e.what());
  // }

  // ESP_LOGI(kTag, "Ready to read from uart.");
  // for (int i = 0; i < 10; ++i) {
  //   auto test = mmrr::uart::Read();
  //   ESP_LOGI(kTag, "%s", fmt::format("uart read: {}", test.c_str()).c_str());
  // }

  // try {
  //   asio::io_context io_context;
  //   asio::ip::tcp::resolver resolver(io_context);
  //   uint16_t port = 54321;
  //   mmrr::client::Server server(io_context, port);
  //   io_context.run();
  // } catch (std::exception& e) {
  //   ESP_LOGE(kTag, "Exception: %s", e.what());
  // }
}

#ifdef __cplusplus
}
#endif