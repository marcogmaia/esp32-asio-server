idf_component_register(
  SRCS
  "./adc.cpp"
  "./main.cpp"
  "./queue.cpp"
  "./uart.cpp"
  # "./client.cpp"
  # "./wifi.cpp"
  # "./alarm/alarm.cpp"

  INCLUDE_DIRS "."

  REQUIRES
  # asio
  # wifi_provisioning
  nvs_flash
  fmt
  esp_adc_cal
  blynk
  semaphore
  board
)