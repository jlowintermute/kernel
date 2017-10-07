cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

project("log" C)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

#add_subdirectory(test)

add_library(log src/log.c)
