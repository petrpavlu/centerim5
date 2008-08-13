include(FindPkgConfig)
include(CheckIncludeFiles)

# {{{ Variable configuration
set(CENTERIM_PROJECT_NAME centerim)
set(CENTERIM_CPPCONSUI_PROJECT_NAME cppconsui)
set(CENTERIM_SRCS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(CENTERIM_CPPCONSUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cppconsui)
set(CENTERIM_PO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/po)
set(CENTERIM_CONFIG_FILENAME config.h)

set(GIT_VERSION 5-devel) # If built from a git repo, set this with describe 

set(CMAKE_BUILD_TYPE RELEASE)

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

# {{{ Check if header found
macro(a_check_header header)
    if(NOT ${header})
        message( FATAL_ERROR ${header} " header not found.")
    endif()
endmacro()    
# }}}

# {{{ Find for executables
a_find_program(GIT_EXECUTABLE git FALSE)
# }}}

# {{{ Find for libraries
a_find_library(LIB_NCURSES ncursesw)
# }}}

# {{{ Checking for system headers
set(CMAKE_REQUIRED_INCLUDES /usr/share/gettext)

CHECK_INCLUDE_FILES(curses.h RENAMED_NCURSES)
CHECK_INCLUDE_FILES(ncurses.h USE_NCURSES)
CHECK_INCLUDE_FILES(gettext.h GETTEXT_H)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${CENTERIM_CONFIG_FILENAME}.in 
    ${CMAKE_CURRENT_BINARY_DIR}/${CENTERIM_CONFIG_FILENAME})
# }}}

# {{{ Check found headers
a_check_header(RENAMED_NCURSES OR USE_NCURSES)
a_check_header(GETTEXT_H)
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
    ${CMAKE_REQUIRED_INCLUDES})

include_directories(${CENTERIM_INCLUDE_DIR})

# Set the main project
project(${CENTERIM_PROJECT_NAME})

