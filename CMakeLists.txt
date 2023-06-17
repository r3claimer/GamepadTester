cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

project(gamepad_test VERSION 1.0)

link_libraries(SDL2 SDL2_gfx)
include_directories(/usr/include/SDL2)

set(CMAKE_CXX_FLAGS "\
 -D_REENTRANT\
 -Wall\
 -Wextra\
 -Wpedantic\
 -g3\
 -ggdb\
")

add_executable(gamepad_test src/main.cpp)
