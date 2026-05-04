cmake_minimum_required(VERSION 3.26)
project(chess-analysis C)

set(CMAKE_C_STANDARD 11)

add_executable(chess-analysis main.c board.c board.h parser.c parser.h panic.c panic.h)