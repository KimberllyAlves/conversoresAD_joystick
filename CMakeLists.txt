

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.0)
set(toolchainVersion 13_3_Rel1)
set(picotoolVersion 2.1.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(conversoresAD_joystick C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(conversoresAD_joystick conversoresAD_joystick.c inc/ssd1306.c)

pico_set_program_name(conversoresAD_joystick "conversoresAD_joystick")
pico_set_program_version(conversoresAD_joystick "0.1")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(conversoresAD_joystick 1)
pico_enable_stdio_usb(conversoresAD_joystick 1)

# Add the standard library to the build
target_link_libraries(conversoresAD_joystick
        pico_stdlib)

# Add the standard include files to the build
target_include_directories(conversoresAD_joystick PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(conversoresAD_joystick 
        hardware_i2c
        hardware_adc
        hardware_pwm
        )

pico_add_extra_outputs(conversoresAD_joystick)

