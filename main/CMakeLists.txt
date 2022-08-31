idf_component_register(
  SRCS
  "./main.cpp"
  "./server.cpp"
  "./wifi_server.cpp"
  "./queue.cpp"
  "./uart.cpp"
  # "alarm/alarm.cpp"

  INCLUDE_DIRS "."

  REQUIRES

  asio
  wifi_provisioning
  nvs_flash
  fmt
  esp_adc_cal
)