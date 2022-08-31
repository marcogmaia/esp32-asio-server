
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

namespace mmrr::queue {

extern QueueHandle_t queue_password;
extern QueueHandle_t queue_adc;

void Init();

}  // namespace mmrr::queue