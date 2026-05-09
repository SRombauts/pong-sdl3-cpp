cmake_minimum_required(VERSION 3.24)

project(pong-sdl3-cpp
    VERSION 0.1.0
    DESCRIPTION "A Pong clone in modern C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# include(CTest) defines BUILD_TESTING (default ON) and enables testing
# at the top level so that ctest --test-dir build picks up the tests.
include(CTest)

add_executable(pong-sdl3-cpp
    src/main.cpp
)

if(MSVC)
    target_compile_options(pong-sdl3-cpp PRIVATE /W4 /permissive-)
else()
    target_compile_options(pong-sdl3-cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(BUILD_TESTING)
    include(FetchContent)

    # Keep doctest lean: do not build its own tests/examples and do not
    # install it. These cache vars must be set before MakeAvailable so the
    # doctest sub-project picks them up.
    set(DOCTEST_WITH_TESTS OFF CACHE BOOL "" FORCE)
    set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "" FORCE)
    set(DOCTEST_NO_INSTALL ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        doctest
        GIT_REPOSITORY https://github.com/doctest/doctest.git
        GIT_TAG v2.4.11
    )
    FetchContent_MakeAvailable(doctest)

    # doctest ships doctest_discover_tests() in scripts/cmake/doctest.cmake.
    list(APPEND CMAKE_MODULE_PATH "${doctest_SOURCE_DIR}/scripts/cmake")
    include(doctest)

    add_subdirectory(tests)
endif()
