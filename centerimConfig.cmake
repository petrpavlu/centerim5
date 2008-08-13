# {{{ Variable configuration
set(CENTERIM_PROJECT_NAME centerim)
set(CENTERIM_CPPCONSUI_PROJECT_NAME cppconsui)
set(CENTERIM_SRCS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CENTERIM_CPPCONSUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cppconsui)
set(CENTERIM_PO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/po)

set(GIT_VERSION 5-devel) # If built from a git repo, set this with describe 

set(CMAKE_BUILD_TYPE RELEASE)

include(FindPkgConfig)

# {{{ Find external utilities
macro(a_find_program var prg req)
    set(required ${req})
    find_program(${var} ${prg})
    if(NOT ${var})
        message(STATUS "${prg} not found.")
        if(required)
            message(FATAL_ERROR "${prg} is required to build awesome")
        endif()
    else()
        message(STATUS "${prg} -> ${${var}}")
    endif()
endmacro()
# }}}

# {{{ A simple macro to find a library and
# check for error.
macro(a_find_library variable library)
    find_library(${variable} ${library})
    if(NOT ${variable})
        message(FATAL_ERROR ${library} " library not found.")
    endif()
endmacro()
# }}}

# {{{ Find for executables
a_find_program(GIT_EXECUTABLE git FALSE)
# }}}

# {{{ Find curses/ncurses(w) libraries
find_library(NCURSESW_LIB NAMES ncursesw)
find_library(CURSESW_LIB NAMES cursesw)
find_library(NCURSES_LIB NAMES ncurses)
find_library(CURSES_LIB NAMES curses)
# }}}

# {{{ Find curses/ncurses(w) header path
find_path(NCURSESW_H NAMES ncursesw.h)
find_path(CURSESW_H NAMES cursesw.h)
find_path(NCURSES_H NAMES ncurses.h)
find_path(CURSES_H NAMES curses.h)
# }}}

# {{{ Find the good curses/ncurses(w) lib/header match
if(NCURSESW_LIB AND NCURSESW_H)
    set(CURSES_LINK_LIB ${NCURSESW_LIB})
    set(CURSES_INCLUDE_DIR ${NCURSESW_H})
    add_definitions(-DUSE_NCURSES)
elseif(NCURSESW_LIB AND NCURSES_H)
    set(CURSES_LINK_LIB ${NCURSESW_LIB})
    set(CURSES_INCLUDE_DIR ${NCURSES_H})
    add_definitions(-DUSE_NCURSES)
elseif(CURSESW_LIB AND CURSESW_H)
    set(CURSES_LINK_LIB ${CURSESW_LIB})
    set(CURSES_INCLUDE_DIR ${CURSESW_H})
elseif(CURSESW_LIB AND CURSES_H) #Can this case happen?
    set(CURSES_LINK_LIB ${CURSESW_LIB})
    set(CURSES_INCLUDE_DIR ${CURSES_H})
elseif(NCURSES_LIB AND NCURSES_H)
    set(CURSES_LINK_LIB ${NCURSES_LIB})
    set(CURSES_INCLUDE_DIR ${NCURSES_H})
    add_definitions(-DUSE_NCURSES)
elseif(CURSES_LIB AND CURSES_H)
    set(CURSES_LINK_LIB ${CURSES_LIB})
    set(CURSES_INCLUDE_DIR ${CURSES_H})
else()
    message(FATAL_ERROR "curses/ncurses(w) not found.")
endif()
# }}}

# {{{ Find gettext header path
find_path(GETTEXT_INCLUDE_DIR
    NAMES gettext.h
    PATHS /usr/share
          /usr/share/gettext)
# }}}

# {{{ Check for required modules to build centerim
pkg_check_modules(CENTERIM_REQUIRED REQUIRED
    glibmm-2.4
    sigc++-2.0
    purple)
# }}}

# {{{ Version stamp
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.git/HEAD AND GIT_EXECUTABLE)
    # get current version
    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_VERSION
        OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()
# }}}

# Configure the git version.
configure_file(${CENTERIM_SRCS_DIR}/git-version.h.in 
    ${CMAKE_CURRENT_BINARY_DIR}/git-version.h @ONLY)

# Set the centerim include dir.
set(CENTERIM_INCLUDE_DIR ${CENTERIM_REQUIRED_INCLUDE_DIRS}
    ${CENTERIM_CPPCONSUI_DIR}
    ${GETTEXT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR} 
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CURSES_INCLUDE_DIR}
    ${GETTEXT_INCLUDE_DIR})

include_directories(${CENTERIM_INCLUDE_DIR})

# Set the main project
project(${CENTERIM_PROJECT_NAME})

