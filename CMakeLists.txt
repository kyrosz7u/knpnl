cmake_minimum_required(VERSION 3.16)
project(KHPNL)

SET(CMAKE_BUILD_TYPE "Debug")

#SET(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_STANDARD 11)
# 设置输出目录
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

include_directories(${PROJECT_SOURCE_DIR})

add_subdirectory(base)
add_subdirectory(event)
add_subdirectory(net)
add_subdirectory(example)
add_subdirectory(unittest)

add_executable(httpserver_test httpserver_test.cpp)
target_link_libraries(httpserver_test http)



