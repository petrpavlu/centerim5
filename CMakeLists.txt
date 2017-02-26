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
  "${centerim5_BINARY_DIR}"
  "${centerim5_SOURCE_DIR}")

# Handle translations.
find_package(Gettext)
mark_as_advanced(GETTEXT_MSGFMT_EXECUTABLE)
mark_as_advanced(GETTEXT_MSGMERGE_EXECUTABLE)

if(GETTEXT_FOUND)
  set(centerim5_LOCALEDIR "${CMAKE_INSTALL_PREFIX}/share/locale")
  set(ENABLE_NLS true)
else()
  message(SEND_ERROR "Gettext not found, translation not possible")
  set(ENABLE_NLS false)
endif()

# Produce documentation.
find_package(Doxygen)

# Checks for tools needed by the termex test framework. Only warn about these
# missing dependencies.
find_program(TIC_EXECUTABLE tic DOC "Path to a program.")
mark_as_advanced(TIC_EXECUTABLE)
if(TIC_EXECUTABLE)
  message(STATUS "Found tic: ${TIC_EXECUTABLE}")
else()
  message(WARNING "tic was not found, running tests will not be possible")
endif()

find_package(PythonInterp 3.5 QUIET)
if(PYTHONINTERP_FOUND)
  message(STATUS
    "Found Python 3: ${PYTHON_EXECUTABLE} (found version \"${PYTHON_VERSION_STRING}\")")
else()
  message(WARNING
    "python >= 3.5 was not found, running tests will not be possible")
endif()

if(TIC_EXECUTABLE AND PYTHONINTERP_FOUND)
  set(TERMEX_TESTS true)
else()
  set(TERMEX_TESTS false)
endif()

# Generate the config file.
set(centerim5_PKGLIBDIR "${CMAKE_INSTALL_PREFIX}/lib/centerim5")
configure_file(config.h.cmake config.h)

install(FILES centerim5.1 DESTINATION share/man/man1)

add_subdirectory(cppconsui)
add_subdirectory(src)
add_subdirectory(tests EXCLUDE_FROM_ALL)
add_subdirectory(plugins)
add_subdirectory(po)
add_subdirectory(doc)
