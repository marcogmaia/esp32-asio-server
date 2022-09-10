#include <algorithm>
#include <array>
#include <optional>
#include <string>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "fmt/format.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gsl/span"
#include "sdkconfig.h"

#include "mmrr/configs.h"
#include "password/password.h"
#include "queue.h"
#include "uart.h"

namespace {

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
}

namespace {

std::optional<char> UartReadByte(TickType_t ticks_to_wait) {
  char byte_read = 0;
  auto read_len  = uart_read_bytes(kUartNum, &byte_read, 1, ticks_to_wait);
  if (read_len < 1) {
    return std::nullopt;
  }
  uart_write_bytes(kUartNum, &byte_read, 1);
  return std::make_optional(byte_read);
}

void UartWriteChar(char ch) {
  uart_write_bytes(kUartNum, &ch, 1);
}

}  // namespace

std::string Read(TickType_t ticks_to_wait) {
  constexpr char kLf  = '\n';
  constexpr char kCr  = '\r';
  auto is_end_of_line = [](char ch) { return ch == kLf || ch == kCr; };

  constexpr int kMaxBufferSize            = 1024;
  std::array<char, kMaxBufferSize> buffer = {0};
  int index                               = 0;

  for (auto byte_read = UartReadByte(ticks_to_wait);
       !is_end_of_line(*byte_read) && index < (kMaxBufferSize - 1);
       byte_read = UartReadByte(ticks_to_wait), ++index) {
    buffer[index] = *byte_read;
  }

  UartWriteChar(kCr);
  UartWriteChar(kLf);
  buffer[index] = '\0';

  return std::string(buffer.data());
}

}  // namespace mmrr::uart
