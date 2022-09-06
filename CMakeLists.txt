cmake_minimum_required(VERSION 3.23)

project(wacc LANGUAGES C)

add_subdirectory(deps/dparser)

add_executable(embed src/embed/embed.c)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
     ${CMAKE_CURRENT_BINARY_DIR}/deps/dparser
)
include(dparser-generate)
include(PrependPath)

set(STR_SRC str.c strtox.c)
prepend_path(STR_SRC src/str STR_SRC_REL)
add_library(str ${STR_SRC_REL})
target_include_directories(str PUBLIC include)

set(WACC_SRC main.c ast.c)
prepend_path(WACC_SRC src/wacc WACC_SRC_REL)
add_executable(wacc ${WACC_SRC_REL})

set(GRAMMAR_G_FILE ${CMAKE_CURRENT_SOURCE_DIR}/resources/grammar.g)
set(GRAMMAR_C_FILE ${CMAKE_CURRENT_BINARY_DIR}/grammar.g.c)
add_custom_command(
  OUTPUT ${GRAMMAR_C_FILE}
  DEPENDS ${GRAMMAR_G_FILE} dparser::make_dparser
  VERBATIM
  COMMAND dparser::make_dparser ${GRAMMAR_G_FILE} -o ${GRAMMAR_C_FILE} -i wacc
)
set_source_files_properties(${GRAMMAR_C_FILE} PROPERTIES GENERATED 1)
target_sources(wacc PRIVATE ${GRAMMAR_C_FILE})

target_include_directories(
  wacc
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
          ${CMAKE_CURRENT_BINARY_DIR}/embedded/include
          ${CMAKE_CURRENT_SOURCE_DIR}/deps/dparser
)
target_link_libraries(wacc PRIVATE str dparser::dparse)
