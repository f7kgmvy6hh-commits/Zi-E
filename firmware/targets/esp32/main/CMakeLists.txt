idf_component_register(
  SRCS "app_main.cpp" "presence_runtime.cpp"
  INCLUDE_DIRS "."
  REQUIRES log)

target_compile_options(${COMPONENT_LIB} PRIVATE
  -fno-exceptions -fno-rtti -Wall -Wextra -Werror)
