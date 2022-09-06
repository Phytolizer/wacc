cmake_minimum_required(VERSION 3.23)

project(c-argparser LANGUAGES C)

add_library(c-argparser repo/arg.c repo/str.c)
target_include_directories(c-argparser PUBLIC repo/include)
add_library(c-argparser::c-argparser ALIAS c-argparser)
