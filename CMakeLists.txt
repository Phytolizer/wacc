cmake_minimum_required(VERSION 3.23)

project(wacc LANGUAGES C)

set(GENERATED_DIR ${CMAKE_CURRENT_BINARY_DIR}/include/wacc/generated)
add_custom_target(
  generated_dir COMMAND ${CMAKE_COMMAND} -E make_directory ${GENERATED_DIR}
)

add_subdirectory(deps/yaep)

add_executable(embed src/embed/embed.c)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(EmbedFile)

add_library(str src/str/str.c)
target_include_directories(str PUBLIC include)

add_executable(wacc src/wacc/main.c)

embed_file(resources/grammar.yaep GRAMMAR_YAEP wacc)

target_include_directories(
  wacc PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
               ${CMAKE_CURRENT_BINARY_DIR}/embedded/include
)
target_link_libraries(wacc PRIVATE str yaep_static)
add_dependencies(wacc generated_dir)
