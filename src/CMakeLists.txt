cmake_minimum_required(VERSION 2.8)

message(STATUS "FIRMWARE ${FIRMWARE}")

if ("${FIRMWARE}" STREQUAL "")
    set(CLIENT_BUILD 1)
    message(STATUS "Building CLIENT software")

else()
    set(FIRMWARE_BUILD 1)
    message(STATUS "Building FIRMWARE")
    # firmware build
    set(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake) # Arduino Toolchain

    set(ARDUINO_DEFAULT_BOARD nano328) # Default Board ID, when not specified
    set(ARDUINO_DEFAULT_PORT /dev/ttyUSB0)
endif()

#====================================================================#
#  Setup Project                                                     #
#====================================================================#
project(SSRBoard C CXX)

#print_board_list()
#print_programmer_list()

if(FIRMWARE_BUILD)
    generate_arduino_library(dimmer
        SRCS zero_cross.cpp dimmer.cpp channels.cpp frac.cpp debug.h list.h
        HDRS zero_cross.h dimmer.h channels.h frac.h debug.cpp list.cpp
    )

    generate_arduino_firmware(ssr-dimmer
        SRCS main.cpp
        LIBS dimmer
    )
endif()