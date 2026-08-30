cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED ZIE_VERIFIED_BOARD_PROFILE OR ZIE_VERIFIED_BOARD_PROFILE STREQUAL "")
  message(FATAL_ERROR
    "BOARD_BINDING_DISABLED: set ZIE_VERIFIED_BOARD_PROFILE only after physical STM32 review")
endif()
if(NOT EXISTS "${ZIE_VERIFIED_BOARD_PROFILE}")
  message(FATAL_ERROR "Verified STM32 board profile does not exist")
endif()
file(READ "${ZIE_VERIFIED_BOARD_PROFILE}" ZIE_BOARD_PROFILE_CONTENT)
if(NOT ZIE_BOARD_PROFILE_CONTENT MATCHES "ZIE_BOARD_PROFILE_VERIFIED=TRUE")
  message(FATAL_ERROR "STM32 board profile is not explicitly verified")
endif()
if(NOT CMAKE_C_COMPILER MATCHES "arm-none-eabi-gcc")
  message(FATAL_ERROR "STM32 target requires the STM32CubeCLT-managed GNU Arm compiler")
endif()

project(zie_stm32_target LANGUAGES C CXX ASM)
message(FATAL_ERROR
  "BOARD_BINDING_DISABLED: production STM32 composition awaits verified MCU/package/clock/pin evidence")

