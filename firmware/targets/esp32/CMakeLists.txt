cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED ZIE_VERIFIED_BOARD_PROFILE OR ZIE_VERIFIED_BOARD_PROFILE STREQUAL "")
  message(FATAL_ERROR
    "BOARD_BINDING_DISABLED: set ZIE_VERIFIED_BOARD_PROFILE only after physical board review")
endif()
if(NOT EXISTS "${ZIE_VERIFIED_BOARD_PROFILE}")
  message(FATAL_ERROR "Verified ESP32 board profile does not exist")
endif()
file(READ "${ZIE_VERIFIED_BOARD_PROFILE}" ZIE_BOARD_PROFILE_CONTENT)
if(NOT ZIE_BOARD_PROFILE_CONTENT MATCHES "ZIE_BOARD_PROFILE_VERIFIED=TRUE")
  message(FATAL_ERROR "ESP32 board profile is not explicitly verified")
endif()
if(NOT DEFINED ENV{IDF_PATH} OR NOT EXISTS "$ENV{IDF_PATH}/tools/cmake/project.cmake")
  message(FATAL_ERROR "ESP-IDF environment is not active")
endif()

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(zie_esp32_target)

