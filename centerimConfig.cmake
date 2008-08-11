# Variable configuration
set( CENTERIM_PROJECT_NAME centerim )
set( CENTERIM_CPPCONSUI_PROJECT_NAME cppconsui )
set( CENTERIM_SRCS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src )
set( CENTERIM_CPPCONSUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cppconsui )
set( CENTERIM_PO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/po )
set( GIT_VERSION 5 ) # TODO: Set this with the git-version util.
set( CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS true )
set( GetText_REQUIRED true )

# A simple macro to find a library and
# check for errors.
macro( a_find_library variable library )
    find_library( ${variable} ${library} )
    if( NOT ${variable} )
        message( FATAL_ERROR ${library} " library not found." )
    endif()
endmacro()
 
a_find_library( LIB_NCURSES ncursesw )                                                                               

include( FindPkgConfig )

find_package( Gettext REQUIRED )
find_path( GETTEXT_INCLUDE_DIR NAMES gettext.h
   PATHS /usr/share
         /usr/share/gettext )

GETTEXT_CREATE_TRANSLATIONS( ${CENTERIM_PO_DIR}/POTFILES.in en_EN )

# Check for required modules to build centerim
pkg_check_modules( CENTERIM_REQUIRED REQUIRED
    glibmm-2.4
    sigc++-2.0
    purple )

# Configure the git version.
configure_file( ${CENTERIM_SRCS_DIR}/git-version.h.in ${CENTERIM_SRCS_DIR}/git-version.h @ONLY )

# Set the centerim include dir.
set( CENTERIM_INCLUDE_DIR ${CENTERIM_REQUIRED_INCLUDE_DIRS}
    ${CENTERIM_CPPCONSUI_DIR}
    ${GETTEXT_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR} 
    )

include_directories( ${CENTERIM_INCLUDE_DIR} )

# Set the main project
project( ${CENTERIM_PROJECT_NAME} )

