cmake_minimum_required(VERSION 3.23)

project(hedley LANGUAGES C)

add_library(hedley INTERFACE)
target_include_directories(hedley INTERFACE repo)
add_library(hedley::hedley ALIAS hedley)
