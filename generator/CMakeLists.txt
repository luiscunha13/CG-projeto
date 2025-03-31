cmake_minimum_required(VERSION 3.5)

# Project Name
PROJECT(generator)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

include_directories(../common/include)
include_directories(include/)

add_executable(${PROJECT_NAME} src/main.cpp
        ../common/include/vertex.h
        include/generator.h
        src/generator.cpp)