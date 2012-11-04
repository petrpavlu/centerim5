#####################################################################
##              handling of package config libraries               ##
#####################################################################

find_package(PkgConfig)
pkg_check_modules(PURPLE REQUIRED "purple >= 2.7.0")
pkg_check_modules(GLIB2 REQUIRED "glib-2.0 >= 2.16.0")
# extaction plugin requires a newer version of glib, check if it's available
pkg_check_modules(GLIB232 QUIET "glib-2.0 >= 2.32.0")
if (NOT GLIB232_FOUND)
  message(STATUS
    "Cannot find glib version >= 2.32, extaction plugin disabled")
endif (NOT GLIB232_FOUND)
pkg_check_modules(SIGC REQUIRED "sigc++-2.0 >= 2.2.0")

#####################################################################
##              handling of (n)curses wide character               ##
#####################################################################

set(Curses_FIND_REQUIRED TRUE)
find_package(Cursesw)

#####################################################################
##                  populating include directories                 ##
#####################################################################

include_directories(
  ${CURSES_INCLUDE_DIRS}
  ${GLIB2_INCLUDE_DIRS}
  ${PURPLE_INCLUDE_DIRS}
  ${SIGC_INCLUDE_DIRS}
  ${centerim5_BINARY_DIR}
  ${centerim5_SOURCE_DIR})

#####################################################################
##                    coping with translations                     ##
#####################################################################

find_package(Gettext)

if (GETTEXT_FOUND)
  set(centerim5_LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
  set(ENABLE_NLS TRUE)
else (GETTEXT_FOUND)
  message(SEND_ERROR "Gettext not found, translation not possible")
  set(ENABLE_NLS FALSE)
endif (GETTEXT_FOUND)

#####################################################################
##                   coping with documentation                     ##
#####################################################################

find_package(Doxygen)

#####################################################################
##                   generation of config file                     ##
#####################################################################

set(centerim5_PKGLIBDIR ${CMAKE_INSTALL_PREFIX}/lib/centerim5)
configure_file(config.h.cmake config.h)
