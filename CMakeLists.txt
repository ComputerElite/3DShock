cmake_minimum_required(VERSION 3.7)
project(3dsTestGraphics)
find_package(nlohmann_json 3.11.3 REQUIRED)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        source/main.cpp
        source/network.cpp
        source/network.hpp
source/json.hpp)

include_directories($ENV{DEVKITARM}/include/)
include_directories($ENV{DEVKITARM}/arm-none-eabi/include)
include_directories($ENV{DEVKITPRO}/libctru/include)
include_directories($ENV{DEVKITARM}/arm-none-eabi/include/c++/*)

add_executable(3DSBuildTemplate ${SOURCE_FILES})
