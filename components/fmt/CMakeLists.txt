# set(COMPONENT_SRCDIRS
# "fmt"
# )

# set(COMPONENT_ADD_INCLUDEDIRS
# "."
# )

# target_compile_definitions(${COMPONENT_TARGET} PUBLIC -DESP32)
# target_compile_options(${COMPONENT_TARGET} PRIVATE -fno-rtti)

idf_component_register(
  SRCS
  "./fmt/format.cc"
  "./fmt/ostream.cc"
  "./fmt/posix.cc"
  "./fmt/printf.cc"

  INCLUDE_DIRS "."

  REQUIRES
  # fmt
)

target_compile_definitions(${COMPONENT_LIB} PUBLIC -DESP32)
target_compile_options(${COMPONENT_LIB} PRIVATE -fno-rtti)