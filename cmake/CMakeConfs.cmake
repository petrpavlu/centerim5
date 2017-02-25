# Find required libraries.
find_package(PkgConfig)
pkg_check_modules(PURPLE REQUIRED "purple >= 2.9.0")
pkg_check_modules(GLIB2 REQUIRED "glib-2.0 >= 2.32.0")
pkg_check_modules(SIGC REQUIRED "sigc++-2.0 >= 2.2.0")
pkg_check_modules(NCURSESW REQUIRED "ncursesw >= 5.8")

# Populate common include directories.
include_directories(
  ${centerim5_BINARY_DIR}
  ${centerim5_SOURCE_DIR})

# Handle translations.
find_package(Gettext)

if(GETTEXT_FOUND)
  set(centerim5_LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")
  set(ENABLE_NLS true)
else()
  message(SEND_ERROR "Gettext not found, translation not possible")
  set(ENABLE_NLS false)
endif()

# Produce documentation.
find_package(Doxygen)

# Generate the config file.
set(centerim5_PKGLIBDIR "${CMAKE_INSTALL_PREFIX}/lib/centerim5")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/config.h")
