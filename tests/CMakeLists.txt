add_executable(pong-sdl3-cpp-tests
    ${PONG_SRC}
    main.cpp
    SmokeTest.cpp
    ApplicationTest.cpp
    FrameTimingTest.cpp
)

# Per-target settings mirror those on pong-sdl3-cpp in the root CMakeLists.txt --
# kept visually parallel so a reviewer can spot drift between the two targets in
# one glance (see issue #12).
target_include_directories(pong-sdl3-cpp-tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(pong-sdl3-cpp-tests PRIVATE doctest::doctest SDL3::SDL3)

# Mirror the FILE_SET HEADERS attached to pong-sdl3-cpp so the test target's
# IDE project also lists the production headers next to their .cpp files.
# BASE_DIRS uses CMAKE_SOURCE_DIR because PONG_INC entries point at <root>/src/
# which lies outside this directory's default base (CMAKE_CURRENT_SOURCE_DIR
# is `tests/` here).
target_sources(pong-sdl3-cpp-tests
    PRIVATE
    FILE_SET pong_headers TYPE HEADERS
    BASE_DIRS ${CMAKE_SOURCE_DIR}/src
    FILES ${PONG_INC}
)

if(MSVC)
    target_compile_options(pong-sdl3-cpp-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(pong-sdl3-cpp-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Same rationale as for pong-sdl3-cpp: with SDL_SHARED=ON, the Windows loader
# needs SDL3.dll next to the test executable to resolve the import table at
# process start, even when no SDL function is actually called by the tests.
if(WIN32)
    add_custom_command(TARGET pong-sdl3-cpp-tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:pong-sdl3-cpp-tests>
        VERBATIM
    )
endif()

# Register each TEST_CASE as an individual CTest entry.
doctest_discover_tests(pong-sdl3-cpp-tests)
