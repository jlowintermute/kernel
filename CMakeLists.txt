cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project("kernel" CXX)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(link_flags -static-libgcc -static-libstdc++)
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(3pp/log)
include_directories(3pp/log/include)
include_directories(include)

add_library(kernel src/Kernel.cpp)
target_link_libraries(kernel
  Log
  ${link_flags}
  Threads::Threads
  )
