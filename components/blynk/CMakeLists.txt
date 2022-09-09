idf_component_register(
  SRC_DIRS
  "./src"
  "./src/utility"

  INCLUDE_DIRS
  "./src"

  REQUIRES
  arduino
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC -DESP32)