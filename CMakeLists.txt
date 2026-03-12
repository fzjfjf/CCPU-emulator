cmake_minimum_required(VERSION 4.1)
project(CCPU_simulator C)

set(CMAKE_C_STANDARD 23)

add_compile_options(-Wall -Wextra -Wpedantic -Werror -Wshadow -Wconversion)

add_executable(CCPU_simulator src/main.c)
