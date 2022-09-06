cmake_minimum_required(VERSION 3.23)

project(wacc LANGUAGES C)

find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/wacc/generated)
add_custom_target(
  generated_dir COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
)

add_library(str src/str/str.c)
target_include_directories(str PUBLIC include)

add_executable(wacc src/wacc/main.c)
flex_target(
  wacc_lexer resources/flex/c.l ${CMAKE_CURRENT_BINARY_DIR}/c.flex.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/include/wacc/generated/c.flex.h
)
target_sources(wacc PRIVATE ${FLEX_wacc_lexer_OUTPUTS})
bison_target(
  wacc_parser resources/bison/c.y ${CMAKE_CURRENT_BINARY_DIR}/c.bison.c
  DEFINES_FILE ${CMAKE_CURRENT_BINARY_DIR}/include/wacc/generated/c.bison.h
)
target_sources(wacc PRIVATE ${BISON_wacc_parser_OUTPUTS})
target_include_directories(wacc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include)
target_link_libraries(wacc PRIVATE str)
add_dependencies(wacc generated_dir)
