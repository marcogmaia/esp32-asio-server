idf_component_register(
  SRCS
  "impl/blynk.cpp"
  "impl/init.cpp"
  "impl/pwm.cpp"

  INCLUDE_DIRS
  "."

  PRIV_REQUIRES
  blynk
  board
)
