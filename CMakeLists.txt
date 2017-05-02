cmake_minimum_required(VERSION 3.7)
project(untitled)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES server.c csapp.c parser_headers.c parser_headers.h log.c log.h server.h helpers.c helpers.h thread_file_write.c)
add_executable(untitled ${SOURCE_FILES})