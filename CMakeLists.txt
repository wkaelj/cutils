cmake_minimum_required(VERSION 3.0.0)

set(C_STANDARD c2x)

file(GLOB SRC ${PROJECT_SOURCE_DIR}/*.c)

add_library(cutils ${SRC})