cmake_minimum_required(VERSION 3.15)

project(run-cppcheck)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

if (UNIX AND NOT APPLE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static -static-libgcc -static-libstdc++")
endif ()

set(SOURCE_FILES main.cpp config.cpp)

add_executable(run-cppcheck main.cpp config.cpp)
install(TARGETS run-cppcheck)

