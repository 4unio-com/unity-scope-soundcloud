
# Install the scope ini file
install(
  FILES "com.ubuntu.scopes.soundcloud_soundcloud.ini"
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

