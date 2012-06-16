#####################################################################
##              handling of package config libraries               ##
#####################################################################

find_package( PkgConfig)
pkg_check_modules(GLIB2 glib-2.0)
pkg_check_modules(PURPLE purple)
pkg_check_modules(SIGC sigc++-2.0)

#####################################################################
##              handling of (n)curses wide character               ##
#####################################################################

set( CURSES_NEED_WIDE TRUE)
find_package( Cursesw)

# as the curses script delivers not the real wide character include
# directory, we now have to extract the right information

get_filename_component(	CURSESW_DIR
			${CURSES_HAVE_NCURSESW_CURSES_H}
			PATH
		)
get_filename_component(	NCURSESW_DIR
			${CURSES_HAVE_NCURSESW_NCURSES_H}
			PATH
		)

if ( IS_DIRECTORY ${CURSESW_DIR})

	set( CURSES_INCLUDE_DIR ${CURSESW_DIR})

else ( IS_DIRECTORY ${CURSESW_DIR})

	if ( IS_DIRECTORY ${NCURSESW_DIR})

		set( CURSES_INCLUDE_DIR ${NCURSESW_DIR})

	else ( IS_DIRECTORY ${NCURSESW_DIR})

		message( SEND_ERROR "No widecharacter version of curses")

	endif ( IS_DIRECTORY ${NCURSESW_DIR})

endif ( IS_DIRECTORY ${CURSESW_DIR})

#####################################################################
##                  populating include directories                 ##
#####################################################################

include_directories(	${CURSES_INCLUDE_DIR}
			${GLIB2_INCLUDE_DIRS}
			${PURPLE_INCLUDE_DIRS}
			${SIGC_INCLUDE_DIRS}
			${centerim_BINARY_DIR}
			${centerim_SOURCE_DIR}
		)

#####################################################################
##                    coping with translations                     ##
#####################################################################

find_package( Gettext)

if (GETTEXT_FOUND)
	message( STATUS "-- Found Gettext: " ${GETTEXT_MSGMERGE_EXECUTABLE} " " ${GETTEXT_MSGFMT_EXECUTABLE} )
	set( centerim_LOCALE_DIR \"${CMAKE_INSTALL_PREFIX}/share/locale/\" )
	set( ENABLE_NLS true )
else (GETTEXT_FOUND)
	message( SEND_ERROR "-- Gettext not found, translation not possible")
	set( ENABLE_NLS false )
endif (GETTEXT_FOUND)

#####################################################################
##                   coping with documentation                     ##
#####################################################################

find_package( Doxygen)

#####################################################################
##                   generation of config file                     ##
#####################################################################

configure_file(	config.in.h config.h)
