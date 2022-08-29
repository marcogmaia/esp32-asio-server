#include <algorithm>
#include <array>
#include <string>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "fmt/format.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gsl/span"
#include "sdkconfig.h"

#include "board_configs.h"
#include "password/password.h"
#include "queue.h"
#include "uart.h"

namespace {

using mmrr::pass::Password;

constexpr const char *kTag = "UART TEST";

constexpr size_t kBufferSize   = 256;
constexpr uart_port_t kUartNum = UART_NUM_0;

constexpr gpio_num_t kTxPin  = GPIO_NUM_1;
constexpr gpio_num_t kRxPin  = GPIO_NUM_3;
constexpr gpio_num_t kRtsPin = GPIO_NUM_NC;
constexpr gpio_num_t kCtsPin = GPIO_NUM_NC;

template <typename T>
constexpr int StrLen(const T *str) {
  const T *begin = str;
  const T *end   = begin;
  while (*end != '\0') {
    ++end;
  }
  return end - begin;
}

void ReadPasswordFromUart() {
  constexpr int password_size = 6;
  std::array<uint8_t, password_size> buffer{};
  uart_read_bytes(kUartNum, buffer.data(), password_size, portMAX_DELAY);
  auto pass = Password{buffer};
  xQueueOverwrite(mmrr::queue::queue_password, &pass);
}

Password GetPassword() {
  Password pass;
  while (xQueueReceive(mmrr::queue::queue_password, &pass, portMAX_DELAY) == pdFALSE) {
  }
  return pass;
}

void UartTask(void *ignore) {
  /* Configure parameters of an UART driver,
   * communication pins and install the driver */
  mmrr::queue::Init();
  mmrr::uart::Init();
  ESP_LOGI(kTag, "Initialized.");

  // Configure a temporary buffer for the incoming data
  // std::array<uint8_t, kBufferSize> buffer;

  const auto expected_pass = Password(std::string_view("654321"));
  Blink();
  while (true) {
    // const auto password = ReadPasswordFromUart();
    ReadPasswordFromUart();
    auto password = GetPassword();

    if (expected_pass == password.get_pass()) {
      ESP_LOGI(kTag, "Correct password!");
    }

    ESP_LOGI(kTag,
             "%s",
             fmt::format("Pass received: {}. fmtlib", password.GetPassAsConstChar()).c_str());
    Blink();
  }
}

}  // namespace

namespace mmrr::uart {

void Init() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;
  uart_config_t uart_config{
      115200,
      UART_DATA_8_BITS,
      UART_PARITY_DISABLE,
      UART_STOP_BITS_1,
      UART_HW_FLOWCTRL_DISABLE,
      122,
      {uart_sclk_t::UART_SCLK_REF_TICK},
  };

  // int intr_alloc_flags = ESP_INTR_FLAG_IRAM;
  int intr_alloc_flags = 0;
  ESP_ERROR_CHECK(uart_driver_install(kUartNum, kBufferSize * 2, 0, 0, nullptr, intr_alloc_flags));
  ESP_ERROR_CHECK(uart_param_config(kUartNum, &uart_config));
  ESP_ERROR_CHECK(uart_set_pin(kUartNum, kTxPin, kRxPin, kRtsPin, kCtsPin));

  xTaskCreatePinnedToCore(
      UartTask, "uart_echo_task", configMINIMAL_STACK_SIZE * 5, nullptr, 10, nullptr, APP_CPU_NUM);
}

}  // namespace mmrr::uart

// extern "C" void app_main() {
//   mmrr::uart::InitTask();
// }