#pragma once

#include <string>

#include "freertos/portmacro.h"

namespace mmrr::uart {

void Init();

/// @brief Blocking read.
/// @return The string read from uart.
std::string Read(TickType_t ticks_to_wait = portMAX_DELAY);

}  // namespace mmrr::uart