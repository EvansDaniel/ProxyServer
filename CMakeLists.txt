cmake_minimum_required(VERSION 3.7)
project(untitled)

set(CMAKE_C_STANDARD 11)


add_definitions(-Wall -std=c++0x)

set(SOURCE_FILES csapp.c server.c parser_headers.c parser_headers.h log.c log.h server.h helpers.c helpers.h log.h)
add_executable(untitled ${SOURCE_FILES})