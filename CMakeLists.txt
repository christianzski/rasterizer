cmake_minimum_required(VERSION 3.5)
project(rasterizer)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_RELEASE "-O2")
set(CMAKE_CXX_FLAGS_DEBUG "-Wall -Wextra -Werror")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SOURCE_FILES
  src/main.cpp
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
