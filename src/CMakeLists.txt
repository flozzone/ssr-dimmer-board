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
project(ArduinoExample C CXX)

print_board_list()
print_programmer_list()

generate_arduino_library(zero_cross
    SRCS zero_cross.cpp
    HDRS zero_cross.h
    BOARD uno)


generate_arduino_firmware(blink
    SRCS blink.cpp
    LIBS zero_cross
    BOARD uno
    PORT /dev/ttyUSB0
)


set(blink_PROGRAMMER avrisp)
