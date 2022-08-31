#include "queue.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "password/password.h"

namespace mmrr::queue {

QueueHandle_t queue_password;
QueueHandle_t queue_adc;

void Init() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized    = true;
  queue_password = xQueueCreate(1, sizeof(mmrr::pass::Password));
  queue_adc      = xQueueCreate(1, sizeof(uint32_t));
}

}  // namespace mmrr::queue