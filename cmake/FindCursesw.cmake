# find the wide curses include file and library
#
#  CURSES_FOUND - system has curses
#  CURSES_LIBRARIES - the libraries needed to use curses
#  CURSES_INCLUDE_DIRS - the include directories needed to use curses
#  CURSES_USE_NCURSES - ncurses found

find_library(CURSES_LIBRARIES ncursesw)
# message(STATUS "NCURSES! " ${CURSES_LIBRARIES})

if (CURSES_LIBRARIES)
  # ncurses found
  set(CURSES_USE_NCURSES true)

else (CURSES_LIBRARIES)
  set(CURSES_USE_NCURSES false)

  find_library(CURSES_LIBRARIES cursesw)
  # message(STATUS "CURSES! " ${CURSES_LIBRARIES})
endif (CURSES_LIBRARIES)

# make sure there is the cursesw.h file available in some include directory
find_path(CURSES_INCLUDE_DIRS cursesw.h)

# handle the QUIETLY and REQUIRED arguments and set CURSES_FOUND to true if
# all listed variables are true
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Curses DEFAULT_MSG CURSES_LIBRARIES
  CURSES_INCLUDE_DIRS)

mark_as_advanced(
  CURSES_LIBRARIES
  CURSES_INCLUDE_DIRS
  CURSES_USE_NCURSES)
