cmake_minimum_required(VERSION 3.23)

project(wacc LANGUAGES C)

add_library(str src/str/str.c)
target_include_directories(str PUBLIC include)
add_library(str::str ALIAS str)

add_executable(wacc src/wacc/main.c)

target_include_directories(
  wacc
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
          ${CMAKE_CURRENT_BINARY_DIR}/embedded/include
          ${CMAKE_CURRENT_SOURCE_DIR}/deps/dparser
)
target_link_libraries(wacc PRIVATE str::str)
