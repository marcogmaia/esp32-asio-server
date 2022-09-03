#include "queue.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "password/password.h"

namespace mmrr::queue {

SemaphoreHandle_t semaphore_password;

QueueHandle_t queue_password;
QueueHandle_t queue_adc;

void Init() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized        = true;
  semaphore_password = xSemaphoreCreateBinary();

  queue_password = xQueueCreate(1, sizeof(bool));
  queue_adc      = xQueueCreate(1, sizeof(uint32_t));
}

}  // namespace mmrr::queue