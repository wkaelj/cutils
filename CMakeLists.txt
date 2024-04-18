cmake_minimum_required(VERSION 3.5.0)

project(cutils LANGUAGES C)

set(CMAKE_C_STANDARD 23)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.c)

add_library(${PROJECT_NAME} STATIC ${SRC})

target_include_directory(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR})
