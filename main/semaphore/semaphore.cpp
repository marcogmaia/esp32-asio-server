#include "semaphore.h"

#include <algorithm>
#include <string>

#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

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
  ESP_LOGI(kTag, "Countdown started.");

  auto init_time = xTaskGetTickCount();
  int wait_ms    = 20;

  for (; time_ms > 0; time_ms -= wait_ms) {
    int ditig_to_show = std::clamp(time_ms / 1000, 0, 9);
    ShowDigit(ditig_to_show);
    vTaskDelayUntil(&init_time, pdMS_TO_TICKS(20));
  }

  ESP_LOGI(kTag, "Countdown ended.\n");
}

void TaskBuzzer(void* ignore) {}

template <typename TaskFunction, typename Param = std::nullptr_t>
class Task {
  Task(TaskFunction&& task,
       std::string_view name,
       Param param         = nullptr,
       uint32_t stack_size = 3 * configMINIMAL_STACK_SIZE)
      : task_(task), param_(param) {
    xTaskCreatePinnedToCore(
        std::forward<Task>(task), name.data(), stack_size, param, 5, handle_, APP_CPU_NUM);
  }

 private:
  TaskFunction task_;
  TaskHandle_t handle_ = nullptr;
};

void Fsm() {
  static State state = State::Green;
  switch (state) {
    case State::Green: {
      Countdown(GetSemaphoreTiming().green);
      break;
    }
    case State::Yellow: {
      Countdown(GetSemaphoreTiming().yellow);
      break;
    }
    case State::Red: {
      // - Create a task to count the cars and update the blynk.
      // - Activate buzzer task
      Countdown(GetSemaphoreTiming().red);
      break;
    }
  }
}

}  // namespace mmrr::semaphore
