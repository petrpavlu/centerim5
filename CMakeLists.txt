project(centerim5)

# CMAKE_CXX_STANDARD requires at least CMake 3.1.
cmake_minimum_required(VERSION 3.1)

# C++ code requires C++11.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS false)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Libtermkey requires C99.
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_EXTENSIONS false)
set(CMAKE_C_STANDARD_REQUIRED true)

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

install(FILES centerim5.1 DESTINATION share/man/man1)

add_subdirectory(cppconsui)
add_subdirectory(src)
add_subdirectory(tests EXCLUDE_FROM_ALL)
add_subdirectory(plugins)
add_subdirectory(po)
add_subdirectory(doc)
