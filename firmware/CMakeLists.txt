cmake_minimum_required(VERSION 3.16)
project(zie_firmware_architecture LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(zie_firmware
    src/ControllerLinkSession.cpp
    src/DeviceIdentity.cpp
    src/ExtensionManifest.cpp
    src/ExtensionRegistry.cpp
    src/HardwareProfile.cpp
    src/MotionCommandLifecycle.cpp
    src/RobotServices.cpp
    src/SafeRobotCommands.cpp
    src/TransactionalConfiguration.cpp
    drivers/stub/BenchHardware.cpp
    profiles/BenchMinimalProfile.cpp
)
target_include_directories(zie_firmware PUBLIC include .)

add_executable(zie_architecture_tests
    tests/architecture_tests.cpp
    tests/device_identity_tests.cpp
    tests/extension_manifest_tests.cpp
    tests/extension_registry_tests.cpp
    tests/transactional_configuration_tests.cpp
)
target_link_libraries(zie_architecture_tests PRIVATE zie_firmware)

enable_testing()
add_test(NAME zie_architecture_tests COMMAND zie_architecture_tests)
