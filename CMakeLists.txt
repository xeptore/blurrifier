cmake_minimum_required(VERSION 3.20.1)

set(CMAKE_C_COMPILER /usr/bin/clang)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

project(blurrifier VERSION 0.1.0)

add_executable(blurrifier main.c)

target_link_libraries (blurrifier jpeg m pthread)
