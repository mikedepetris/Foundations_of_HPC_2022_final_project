cmake_minimum_required(VERSION 3.22)
project(gameoflife C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
#It is C settings only, C++ needs CMAKE_CXX_FLAGS
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fopenmp")

add_executable(gameoflife.x gameoflife.h gameoflife.c files_io.h files_io.c new_playground.c evolution_static.c evolution_ordered.c evolution_wave.c evolution_whiteblack.c)
