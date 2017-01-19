#=============================================================================#
# Author: QueezyTheGreat                                                      #
# Date:   26.04.2011                                                          #
#                                                                             #
# Description: Arduino CMake example                                          #
#                                                                             #
#=============================================================================#
set(CMAKE_TOOLCHAIN_FILE cmake/ArduinoToolchain.cmake) # Arduino Toolchain


cmake_minimum_required(VERSION 2.8)
#====================================================================#
#  Setup Project                                                     #
#====================================================================#
project(SSRBoard C CXX)

#print_board_list()
#print_programmer_list()

set(ARDUINO_DEFAULT_BOARD uno) # Default Board ID, when not specified
set(ARDUINO_DEFAULT_PORT /dev/ttyUSB0)

generate_arduino_library(zero_cross
    SRCS zero_cross.cpp
    HDRS zero_cross.h
)


generate_arduino_firmware(blink
    SRCS blink.cpp
    LIBS zero_cross
)
