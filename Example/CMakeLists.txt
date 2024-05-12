cmake_minimum_required(VERSION 3.5.0)
set(CMAKE_CXX_STANDARD 20)

set(PROJECT GuiRebirth)
project(${PROJECT})

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)
add_library(${PROJECT} SHARED ${SOURCE_FILES})
