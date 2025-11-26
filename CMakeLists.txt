cmake_minimum_required(VERSION 3.14)
project(CP1GroupProject LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/release-1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_library(student_logic STATIC src/student.c)
target_include_directories(student_logic PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(app src/main.c)
target_link_libraries(app PRIVATE student_logic)

# Tests
file(GLOB TEST_SOURCES tests/*.cxx)
add_executable(unit_tests ${TEST_SOURCES})
target_link_libraries(unit_tests PRIVATE student_logic gtest gtest_main)
add_test(NAME unit_tests COMMAND unit_tests)
