
function(configure_scope_files)
  foreach(_file ${ARGV})
    configure_file(
      "${_file}.in"
      "${CMAKE_CURRENT_BINARY_DIR}/${_file}"
      @ONLY
    )
  endforeach()
endfunction()

configure_scope_files(
  manifest.json
  soundcloud.provider
  soundcloud.service
  soundcloud.application
)

# Make this file show up in QtCreator
add_custom_target(hidden_files
  ALL
  SOURCES
    "soundcloud.apparmor"
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/manifest.json"
    "soundcloud.apparmor"
    "${CMAKE_CURRENT_BINARY_DIR}/soundcloud.provider"
    "${CMAKE_CURRENT_BINARY_DIR}/soundcloud.service"
    "${CMAKE_CURRENT_BINARY_DIR}/soundcloud.application"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)

install(
  DIRECTORY "qml-plugin"
  DESTINATION ${CMAKE_INSTALL_PREFIX}
)
