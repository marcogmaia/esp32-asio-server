
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "mmrr/configs.h"

namespace mmrr::semaphore {

namespace {

constexpr const char* kTag = "Semaforo";

}

enum class State {
  Red,
  Yellow,
  Green,
};

template <typename T>
constexpr bool IsNumberInRange(T num, T low, T high) {
  return low <= num && num <= high;
}

void ShowDigit(int digit) {
  if (!IsNumberInRange(digit, 0, 9)) {
    ESP_LOGE(kTag, "Digit out of bounds.");
    return;
  }
  constexpr int num_array[10][7] = {{1, 1, 1, 1, 1, 1, 0},   // 0
                                    {0, 1, 1, 0, 0, 0, 0},   // 1
                                    {1, 1, 0, 1, 1, 0, 1},   // 2
                                    {1, 1, 1, 1, 0, 0, 1},   // 3
                                    {0, 1, 1, 0, 0, 1, 1},   // 4
                                    {1, 0, 1, 1, 0, 1, 1},   // 5
                                    {1, 0, 1, 1, 1, 1, 1},   // 6
                                    {1, 1, 1, 0, 0, 0, 0},   // 7
                                    {1, 1, 1, 1, 1, 1, 1},   // 8
                                    {1, 1, 1, 0, 0, 1, 1}};  // 9
  gpio_set_level(kPinSegA, num_array[digit][0]);
  gpio_set_level(kPinSegB, num_array[digit][1]);
  gpio_set_level(kPinSegC, num_array[digit][2]);
  gpio_set_level(kPinSegD, num_array[digit][3]);
  gpio_set_level(kPinSegE, num_array[digit][4]);
  gpio_set_level(kPinSegF, num_array[digit][5]);
  gpio_set_level(kPinSegG, num_array[digit][6]);
}

void TurnOffDigits() {
  gpio_set_level(kPinSegA, 0);
  gpio_set_level(kPinSegB, 0);
  gpio_set_level(kPinSegC, 0);
  gpio_set_level(kPinSegD, 0);
  gpio_set_level(kPinSegE, 0);
  gpio_set_level(kPinSegF, 0);
  gpio_set_level(kPinSegG, 0);
}

void TaskCountdown(void* ignore) {
  auto last_wake_time = xTaskGetTickCount();
  ESP_LOGI(kTag, "Countdown started.");
  for (int i = 9; i >= 0; --i) {
    ShowDigit(i);
    vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(1000));
  }
  ESP_LOGI(kTag, "Countdown ended.\n");
  vTaskDelete(nullptr);
}

/// @brief Time in seconds.
void StartCountdown(int time) {
  xTaskCreatePinnedToCore(TaskCountdown,
                          "TaskCountdown",
                          2 * configMINIMAL_STACK_SIZE,
                          nullptr,
                          54,
                          nullptr,
                          APP_CPU_NUM);
}

void Fsm() {
  static State state = State::Green;
  switch (state) {
    case State::Green: {
      StartCountdown(4);
      break;
    }
    case State::Yellow: {
      break;
    }
    case State::Red: {
      break;
    }
  }
}

}  // namespace mmrr::semaphore
