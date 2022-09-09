idf_component_register(
  SRCS
  "./blynk/blynk.c"

  INCLUDE_DIRS "."

  REQUIRES
  lwip

)

target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-format-truncation)