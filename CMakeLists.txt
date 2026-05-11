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

include(FetchContent)

# SDL3, doctest, and any other third-party project fetched here may still
# declare compatibility with CMake versions older than 3.5, which CMake 4
# rejects by default. Set the minimum policy version once for every
# FetchContent sub-project to keep the configure step working on CMake 4+.
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# ----------------------------------------------------------------------------
# SDL3 (runtime dependency of the main executable)
# ----------------------------------------------------------------------------
# Link mode: shared. This matches SDL3's upstream default and is the mode
# most pre-built distributions ship. On Windows it produces SDL3.dll, which
# we copy next to the executable in a POST_BUILD step further down.
set(SDL_SHARED ON CACHE BOOL "" FORCE)
set(SDL_STATIC OFF CACHE BOOL "" FORCE)
# We don't need SDL's test programs, examples, or install rules.
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SDL_INSTALL OFF CACHE BOOL "" FORCE)
set(SDL_UNINSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.2.30
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(SDL3)

# Production source list, declared once and consumed by both the shipped
# executable and the test binary. Adding a new non-entry-point source means
# editing this list (and nothing else). Absolute paths via
# CMAKE_CURRENT_SOURCE_DIR let tests/CMakeLists.txt -- which lives one
# directory deeper -- consume the variable unchanged. `main.cpp` is the only
# source kept off this list because each binary supplies its own entry point.
set(PONG_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Application.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/FrameTiming.cpp
)

add_executable(pong-sdl3-cpp ${PONG_SRC} src/main.cpp)

# Per-target settings for pong-sdl3-cpp and pong-sdl3-cpp-tests are deliberately
# duplicated (here and in tests/CMakeLists.txt) rather than centralised in a
# library target -- see issue #12. Keep the two blocks visually similar so a
# reviewer can spot drift at a glance; promote to a helper macro if they grow.
target_include_directories(pong-sdl3-cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(pong-sdl3-cpp PRIVATE SDL3::SDL3)

if(MSVC)
    target_compile_options(pong-sdl3-cpp PRIVATE /W4 /permissive-)
else()
    target_compile_options(pong-sdl3-cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()

# With SDL_SHARED=ON, Windows builds produce SDL3.dll. Copy it next to the
# executable after every build so the program can be launched from the
# build tree without manually adjusting PATH. On Linux/macOS the loader
# finds libSDL3.so / libSDL3.dylib through CMake's RPATH handling.
if(WIN32)
    add_custom_command(TARGET pong-sdl3-cpp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:pong-sdl3-cpp>
        VERBATIM
    )
endif()

if(BUILD_TESTING)
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
