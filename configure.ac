#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Hurray for <http://www-src.lip6.fr/homepages/Alexandre.Duret-Lutz/autotools.html>

AC_PREREQ(2.59)
AC_INIT([centerim], [5.0-alpha2], [http://sourceforge.net/projects/centerim])
# prevent AC_PROG_CC, AC_PROG_CXX from initializing CFLAGS and CXXFLAGS
: ${CFLAGS=""}
: ${CXXFLAGS=""}
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall])

AC_GNU_SOURCE

########################
# Checks for programs. #
########################
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_PROG_LIBTOOL

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

#########################
# Checks for libraries. #
#########################

# libpurple
PKG_CHECK_MODULES([PURPLE], [purple >= 2.4.0])
AC_SUBST(PURPLE_CFLAGS)
AC_SUBST(PURPLE_LIBS)

# glib
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.0.0 gobject-2.0 gmodule-2.0 gthread-2.0])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

# libsigc++
PKG_CHECK_MODULES([SIGC], [sigc++-2.0 >= 2.0.0])
AC_SUBST(SIGC_CFLAGS)
AC_SUBST(SIGC_LIBS)

# TODO -- verify which should be the latest ncurses library supported and check it too
#PKG_CHECK_MODULES([NCURSESW], [ncursesw >= 5.7])
#AC_SUBST(NCURSESW_LIBS)
#AC_SUBST(NCURSESW_CFLAGS)

AC_PATH_LIB_LIBCONFIG
AC_PATH_LIB(ncursesw, , ncurses.h, ncursesw5-config, , , [
	AC_MSG_RESULT(no)
	AC_ERROR(You must have the ncursesw library and developement headers installed to build.)
	])

############################
# Checks for header files. #
############################

##################################################################
# Checks for typedefs, structures, and compiler characteristics. #
##################################################################

#################################
# Checks for library functions. #
#################################

###########
# Options #
###########

AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug],
	[compile with debugging support])])

AS_IF([test "x$enable_debug" = "xyes"],
	[CFLAGS="-ggdb -O0 -Wall -pedantic $CFLAGS"
	 CXXFLAGS="-ggdb -O0 -Wall -pedantic $CXXFLAGS"
	 AC_DEFINE(DEBUG, 1, [Define if debugging is enabled.])],
	[CFLAGS="-O2 -Wall -pedantic $CFLAGS"
	 CXXFLAGS="-O2 -Wall -pedantic $CXXFLAGS"])
AC_SUBST(CFLAGS)
AC_SUBST(CXXFLAGS)

########################
# Create output files. #
########################
AC_CONFIG_FILES([Makefile
		 libtermkey/Makefile
		 cppconsui/Makefile
		 doc/Makefile
		 doc/doxygen-cppconsui.conf
		 doc/doxygen-cim5.conf
		 po/Makefile.in
		 src/Makefile
		 tests/Makefile])
AC_OUTPUT
