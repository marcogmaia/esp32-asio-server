idf_component_register(
  SRCS
  "impl/blynk.cpp"
  "impl/init.cpp"
  "impl/ldr.cpp"
  "impl/pwm.cpp"
  "impl/semaphore.cpp"

  INCLUDE_DIRS
  "."

  PRIV_REQUIRES
  blynk
  board
)
