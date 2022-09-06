macro(embed_file FILEPATH IDENTIFIER TARGET)
  set(filepath ${FILEPATH})
  cmake_path(GET filepath FILENAME NAME)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/embedded/${NAME}.c
    DEPENDS embed ${CMAKE_CURRENT_SOURCE_DIR}/${FILEPATH}
    COMMAND ${CMAKE_COMMAND} -E make_directory
            ${CMAKE_CURRENT_BINARY_DIR}/embedded/include/embedded
    COMMAND
      $<TARGET_FILE:embed> ${CMAKE_CURRENT_SOURCE_DIR}/${FILEPATH}
      ${CMAKE_CURRENT_BINARY_DIR}/embedded/include/embedded/${NAME}.h
      ${CMAKE_CURRENT_BINARY_DIR}/embedded/${NAME}.c ${IDENTIFIER}
  )
  if(TARGET ${TARGET})
    target_sources(
      ${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/embedded/${NAME}.c
    )
    target_include_directories(
      ${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/embedded/include
    )
  endif()
endmacro()
