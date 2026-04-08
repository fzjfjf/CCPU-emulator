cmake_minimum_required(VERSION 4.1)
project(CCPU_simulator C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion)

add_executable(CCPU_simulator src2.0/main.c
        src2.0/input.c
        src2.0/input.h
        src2.0/memory.c
        src2.0/memory.h)
