
# Install the scope ini file
install(
  FILES "com.ubuntu.scopes.soundcloud_soundcloud.ini"
  DESTINATION ${SCOPE_INSTALL_DIR}
)

# Install the scope images
install(
  FILES
    "icon.png"
    "logo.png"
    "screenshot.png"
  DESTINATION
    "${SCOPE_INSTALL_DIR}"
)

