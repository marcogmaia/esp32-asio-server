#include <exception>
#include <iostream>

#include <driver/gpio.h>
#include <esp_log.h>
#include <fmt/format.h>

#include "adc.h"
#include "board_configs.h"
#include "client.h"
#include "queue.h"
#include "semaforo/semaforo.h"
#include "uart.h"

#ifdef __cplusplus
extern "C" {
#endif

constexpr auto kTag = "Main";

void app_main(void) {
  ESP_LOGI(kTag, "ESP_WIFI_MODE_STA");
  mmrr::semaforo::Init();
  mmrr::queue::Init();
  mmrr::uart::Init();
  mmrr::adc::Init();
}

#ifdef __cplusplus
}
#endif