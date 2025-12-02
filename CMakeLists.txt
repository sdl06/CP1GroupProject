cmake_minimum_required(VERSION 3.14)
project(CP1GroupProject LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(app src/main.c)
target_link_libraries(app PRIVATE pthread)
