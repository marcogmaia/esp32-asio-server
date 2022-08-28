#include <array>
#include <string>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "board_configs.h"
#include "uart.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

namespace {

constexpr const char *kTag = "UART TEST";

constexpr size_t kBufferSize = 1024;
constexpr uart_port_t kUartNum = UART_NUM_0;

constexpr gpio_num_t kTxPin = GPIO_NUM_1;
constexpr gpio_num_t kRxPin = GPIO_NUM_3;
constexpr gpio_num_t kRtsPin = GPIO_NUM_NC;
constexpr gpio_num_t kCtsPin = GPIO_NUM_NC;

template <typename T>
constexpr int StrLen(const T *str) {
  const T *begin = str;
  const T *end = begin;
  while (*end != '\0') {
    ++end;
  }
  return end - begin;
}

void echo_task(void *ignore) {
  /* Configure parameters of an UART driver,
   * communication pins and install the driver */

  uart::Init();
  ESP_LOGI(kTag, "Initialized.");

  // Configure a temporary buffer for the incoming data
  std::array<uint8_t, kBufferSize> buffer;

  Blink();
  while (1) {
    // Read data from the UART
    int len = uart_read_bytes(kUartNum, buffer.data(), (kBufferSize - 1), pdMS_TO_TICKS(20));
    // Write data back to the UART
    if (len) {
      buffer[len] = '\0';
      ESP_LOGI(kTag, "Message received: %s", buffer.data());
      // uart_write_bytes(kUartNum, buffer.data(), len);
      Blink();
    }
  }
}

}  // namespace

namespace uart {

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

void InitTask() {
  xTaskCreatePinnedToCore(
      echo_task, "uart_echo_task", configMINIMAL_STACK_SIZE * 5, nullptr, 10, nullptr, APP_CPU_NUM);
}

}  // namespace uart

