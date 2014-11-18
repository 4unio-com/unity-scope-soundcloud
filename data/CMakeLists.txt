
intltool_merge_translations(
  "${CMAKE_CURRENT_SOURCE_DIR}/${SCOPE_NAME}.ini.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${SCOPE_NAME}.ini"
  ALL
  UTF8
)

# Install the scope ini file
install(
  FILES "${CMAKE_CURRENT_BINARY_DIR}/${SCOPE_NAME}.ini"
  DESTINATION ${SCOPE_INSTALL_DIR}
)

# Install the scope images
install(
  FILES
    "comments.png"
    "icon.png"
    "likes.png"
    "logo.png"
    "plays.png"
    "reposts.png"
    "screenshot.png"
  DESTINATION
    "${SCOPE_INSTALL_DIR}"
)
