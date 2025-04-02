cmake_minimum_required(VERSION 3.7)
project(3dsTestGraphics)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        source/main.c)

include_directories($ENV{DEVKITARM}/include/)
include_directories($ENV{DEVKITARM}/arm-none-eabi/include)
include_directories($ENV{DEVKITPRO}/libctru/include)
include_directories($ENV{DEVKITARM}/arm-none-eabi/include/c++/*)

add_executable(3DSBuildTemplate ${SOURCE_FILES})
