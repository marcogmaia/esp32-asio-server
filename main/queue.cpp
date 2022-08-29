#include "queue.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "password/password.h"

namespace mmrr::queue {

QueueHandle_t queue_password;

void Init() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized    = true;
  queue_password = xQueueCreate(1, sizeof(mmrr::pass::Password));
}

}  // namespace mmrr::queue