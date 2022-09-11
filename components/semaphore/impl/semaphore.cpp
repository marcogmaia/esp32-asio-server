#include <algorithm>
#include <string>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#include "impl/ldr.h"
#include "mmrr/configs.h"
#include "mmrr/semaphore.h"

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

void Countdown(int time_ms) {
  ESP_LOGD(kTag, "Countdown started.");

  auto init_time = xTaskGetTickCount();
  int wait_ms    = 20;

  for (; time_ms > 0; time_ms -= wait_ms) {
    int ditig_to_show = std::clamp(time_ms / 1000, 0, 9);
    ShowDigit(ditig_to_show);
    vTaskDelayUntil(&init_time, pdMS_TO_TICKS(20));
  }

  ESP_LOGD(kTag, "Countdown ended.");
}

void TaskBuzzer(void* ignore) {
  if (IsBuzzerOn()) {
    BuzzerTurnOn();
  } else {
    BuzzerTurnOff();
  }

  while (true) {
    if (IsBuzzerStateChanged()) {
      if (IsBuzzerOn()) {
        BuzzerTurnOn();
      } else {
        BuzzerTurnOff();
      }
    }
  }
  vTaskDelay(pdMS_TO_TICKS(100));
}

TaskHandle_t CreateBuzzerTask() {
  TaskHandle_t handle = nullptr;
  xTaskCreatePinnedToCore(
      TaskBuzzer, "TaskBuzzer", configMINIMAL_STACK_SIZE * 2, nullptr, 4, &handle, APP_CPU_NUM);
  return handle;
}

void TaskCarCounter(void* ignore) {
  while (true) {
    if (mmrr::semaphore::LdrRead() < 200) {
      AddCarCounter();
      vTaskDelay(pdMS_TO_TICKS(500));
    } else {
      vTaskDelay(pdMS_TO_TICKS(50));
    }
  }
}

TaskHandle_t CreateCarCounterTask() {
  TaskHandle_t handle = nullptr;
  xTaskCreatePinnedToCore(TaskCarCounter,
                          "TaskCarCounter",
                          configMINIMAL_STACK_SIZE * 2,
                          nullptr,
                          4,
                          &handle,
                          APP_CPU_NUM);
  return handle;
}

void Fsm() {
  static State state = State::Green;

  switch (state) {
    case State::Green: {
      gpio_set_level(kPinLedGreen, 1);
      Countdown(GetSemaphoreTiming().green);
      gpio_set_level(kPinLedGreen, 0);
      state = State::Yellow;
      break;
    }
    case State::Yellow: {
      gpio_set_level(kPinLedYellow, 1);
      Countdown(GetSemaphoreTiming().yellow);
      gpio_set_level(kPinLedYellow, 0);
      state = State::Red;
      break;
    }
    case State::Red: {
      auto buzzer_handle    = CreateBuzzerTask();
      auto car_task_handler = CreateCarCounterTask();

      gpio_set_level(kPinLedRed, 1);
      Countdown(GetSemaphoreTiming().red);
      gpio_set_level(kPinLedRed, 0);

      vTaskDelete(buzzer_handle);
      vTaskDelete(car_task_handler);
      BuzzerTurnOff();

      state = State::Green;
      break;
    }
  }
}

}  // namespace mmrr::semaphore
