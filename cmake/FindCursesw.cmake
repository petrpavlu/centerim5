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

# Some systems (ubuntu 14.10, and others?) Need to include
# /usr/include/ncursesw .  the autotools build discovers this
# via ncursesw5-config --cflags.  Other systems don't have anything
# set for --cflags .
# TODO: better way to determine this?  Other software includes
# cursesw/curses.h instead of curses.h  .  That alone may solve it.
find_program(NCURSESW5_CONFIG ncursesw5-config)

if(NCURSESW5_CONFIG)
  execute_process(COMMAND ${NCURSESW5_CONFIG} --cflags
    OUTPUT_VARIABLE CURSES_CFLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
else()
  set(CURSES_CFLAGS )
endif()

# handle the QUIETLY and REQUIRED arguments and set CURSES_FOUND to true if
# all listed variables are true
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Curses DEFAULT_MSG CURSES_LIBRARIES
  CURSES_INCLUDE_DIRS)

mark_as_advanced(
  CURSES_LIBRARIES
  CURSES_INCLUDE_DIRS
  CURSES_USE_NCURSES
  CURSES_CFLAGS)
