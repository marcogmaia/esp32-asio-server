
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace mmrr::queue {

extern QueueHandle_t queue_password;

void Init();

}  // namespace mmrr::queue