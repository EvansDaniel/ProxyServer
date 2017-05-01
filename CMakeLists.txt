cmake_minimum_required(VERSION 3.7)
project(untitled)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES tiny.c proxy.c csapp.c proxy.h parser_headers.c parser_headers.h)
add_executable(untitled ${SOURCE_FILES})