idf_component_register(
  SRCS
  "main.cpp"
  "server.cpp"
  "wifi_server.cpp"

  INCLUDE_DIRS "."

  REQUIRES
  json
  asio
  wifi_provisioning
  nvs_flash
)