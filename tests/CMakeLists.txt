add_executable(pong-sdl3-cpp-tests
    main.cpp
    SmokeTest.cpp
)

target_link_libraries(pong-sdl3-cpp-tests PRIVATE doctest::doctest)

if(MSVC)
    target_compile_options(pong-sdl3-cpp-tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(pong-sdl3-cpp-tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Register each TEST_CASE as an individual CTest entry.
doctest_discover_tests(pong-sdl3-cpp-tests)
