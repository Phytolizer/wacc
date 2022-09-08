cmake_minimum_required(VERSION 3.23)

project(wacc LANGUAGES C)

add_subdirectory(deps/c-argparser)
add_subdirectory(deps/dparser)
add_subdirectory(deps/hedley)
add_subdirectory(deps/uthash)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake
     ${CMAKE_CURRENT_BINARY_DIR}/deps/dparser
)
include(dparser-generate)
include(PrependPath)

add_library(sum INTERFACE)
target_include_directories(sum INTERFACE include)

set(STR_SRC str.c strtox.c)
prepend_path(STR_SRC src/str STR_SRC_REL)
add_library(str ${STR_SRC_REL})
target_include_directories(str PUBLIC include)

add_library(process src/process/process.c)
target_include_directories(process PUBLIC include)

add_library(file src/file/file.c)
target_include_directories(file PUBLIC include)
target_link_libraries(file PUBLIC str sum)

set(WACC_SRC run.c ast.c codegen.c mkexe.c scan.c)
prepend_path(WACC_SRC src/wacc WACC_SRC_REL)
add_library(wacc ${WACC_SRC_REL})

configure_file(resources/grammar.g include/grammar.g COPYONLY)
set(GRAMMAR_G_FILE ${CMAKE_CURRENT_BINARY_DIR}/include/grammar.g)
set(GRAMMAR_C_FILE ${CMAKE_CURRENT_BINARY_DIR}/grammar.g.c)
add_custom_command(
  OUTPUT ${GRAMMAR_C_FILE}
         ${CMAKE_CURRENT_BINARY_DIR}/include/grammar.g.d_parser.h
  DEPENDS ${GRAMMAR_G_FILE} dparser::make_dparser
  VERBATIM
  COMMAND dparser::make_dparser ${GRAMMAR_G_FILE} -o ${GRAMMAR_C_FILE} -i wacc
          -t 1
)
set_source_files_properties(${GRAMMAR_C_FILE} PROPERTIES GENERATED 1)
target_sources(wacc PRIVATE ${GRAMMAR_C_FILE})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_CURRENT_BINARY_DIR}/include
)

configure_file(cmake/config.h.cmake-in include/wacc-config.h)

target_include_directories(
  wacc
  PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/include
         ${CMAKE_CURRENT_BINARY_DIR}/embedded/include
         ${CMAKE_CURRENT_SOURCE_DIR}/deps/dparser
)
target_link_libraries(
  wacc
  PUBLIC str
         file
         process
         dparser::dparse
         hedley::hedley
         c-argparser::c-argparser
         uthash::uthash
)

add_executable(wacc_driver src/wacc_driver/main.c)
set_target_properties(wacc_driver PROPERTIES OUTPUT_NAME wacc)
target_link_libraries(wacc_driver PRIVATE wacc)

enable_testing()
add_executable(test_wacc tests/test.c)
target_link_libraries(test_wacc PRIVATE wacc)
