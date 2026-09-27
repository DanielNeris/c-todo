cmake_minimum_required(VERSION 3.20)
project(c_todo C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_executable(todo main.c)
target_compile_options(todo PRIVATE -Wall -Wextra)
