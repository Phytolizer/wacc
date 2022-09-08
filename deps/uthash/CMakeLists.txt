cmake_minimum_required(VERSION 3.23)

project(uthash LANGUAGES C)

add_library(uthash INTERFACE)
target_include_directories(uthash INTERFACE repo/include)
add_library(uthash::uthash ALIAS uthash)
