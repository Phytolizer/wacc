cmake_minimum_required(VERSION 3.23)

project(wacc LANGUAGES C)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(PrependPath)

add_subdirectory(deps/packcc)

macro(packcc_file INPUT OUTPUT TARGET)
  add_custom_command(
    OUTPUT ${OUTPUT}.c ${OUTPUT}.h
    COMMAND packcc::packcc -o ${OUTPUT} ${INPUT}
    DEPENDS ${INPUT}
  )
  if(TARGET ${TARGET})
    target_sources(${TARGET} PRIVATE ${OUTPUT}.c)
  endif()
endmacro()

set(STR_SRC str.c strtox.c)
prepend_path(STR_SRC src/str/ STR_SRC_REL)
add_library(str ${STR_SRC_REL})
target_include_directories(str PUBLIC include)
add_library(str::str ALIAS str)

add_library(file src/file/file.c)
target_include_directories(file PUBLIC include)
add_library(file::file ALIAS file)

add_library(process src/process/process.c)
target_include_directories(process PUBLIC include)
add_library(process::process ALIAS process)

set(WACC_SRC run.c ast.c system.c)
prepend_path(WACC_SRC src/wacc/ WACC_SRC_REL)
add_library(wacc ${WACC_SRC_REL})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/include/packcc
  COMMAND ${CMAKE_COMMAND} -E make_directory
          ${CMAKE_CURRENT_BINARY_DIR}/include/packcc
)

packcc_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/grammar.peg
  ${CMAKE_CURRENT_BINARY_DIR}/include/packcc/grammar wacc
)

target_include_directories(
  wacc
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_libraries(
  wacc
  PUBLIC str::str
  PRIVATE file::file
)

add_executable(wacc_driver src/wacc_driver/main.c)
target_link_libraries(wacc_driver PRIVATE wacc)

configure_file(cmake/config.h.cmake-in include/config.h)

enable_testing()
add_executable(wacc_test tests/main.c tests/collect.c)
target_include_directories(
  wacc_test PRIVATE tests/include ${CMAKE_CURRENT_BINARY_DIR}/include
)
target_link_libraries(wacc_test PRIVATE wacc process::process)

add_test(NAME wacc_test COMMAND wacc_test)
