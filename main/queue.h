
#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

namespace mmrr::queue {

extern SemaphoreHandle_t semaphore_password;

extern QueueHandle_t queue_password;
extern QueueHandle_t queue_adc;

void Init();

}  // namespace mmrr::queue