idf_component_register(
  SRCS
  "./adc.cpp"
  "./client.cpp"
  "./main.cpp"
  "./queue.cpp"
  "./uart.cpp"
  "./wifi.cpp"
  "alarm/alarm.cpp"

  INCLUDE_DIRS "."

  REQUIRES

  asio
  wifi_provisioning
  nvs_flash
  fmt
  esp_adc_cal
)