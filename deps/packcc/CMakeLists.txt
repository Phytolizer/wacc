cmake_minimum_required(VERSION 3.23)

project(packcc LANGUAGES C)

add_executable(packcc repo/src/packcc.c)
add_executable(packcc::packcc ALIAS packcc)
