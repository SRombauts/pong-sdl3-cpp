cmake_minimum_required(VERSION 3.24)

project(pong-sdl3-cpp
    VERSION 0.1.0
    DESCRIPTION "A Pong clone in modern C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(pong-sdl3-cpp
    src/main.cpp
)

if(MSVC)
    target_compile_options(pong-sdl3-cpp PRIVATE /W4 /permissive-)
else()
    target_compile_options(pong-sdl3-cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()
