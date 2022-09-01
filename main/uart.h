#pragma once

#include <string>

namespace mmrr::uart {

void Init();

/// @brief Blocking read.
/// @return The string read from uart.
std::string Read();

}  // namespace mmrr::uart