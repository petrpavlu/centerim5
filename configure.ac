# *- Autoconf -*-
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
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

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

# inspired by code found in configure.ac of OpenChange project
AC_CACHE_CHECK(
[whether we are using the Sun C++ compiler],
[ac_cv_cxx_compiler_suncc], [
	AC_LANG_PUSH([C++])
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([], [[
		#ifndef __SUNPRO_CC
			choke me
		#endif
		]])], [ac_compiler_suncc=yes], [ac_compiler_suncc=no])
	AC_LANG_POP([C++])
	ac_cv_cxx_compiler_suncc=$ac_compiler_suncc])

#########################
# Checks for libraries. #
#########################

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

# libpurple
PKG_CHECK_MODULES([PURPLE], [purple >= 2.4.0])
AC_SUBST(PURPLE_CFLAGS)
AC_SUBST(PURPLE_LIBS)

# glib
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.0.0])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

# libsigc++
PKG_CHECK_MODULES([SIGC], [sigc++-2.0 >= 2.0.0])
AC_SUBST(SIGC_CFLAGS)
AC_SUBST(SIGC_LIBS)

# ncursesw
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

###############################
# Set default compiler flags. #
###############################

if test "$GCC" = yes; then
	CFLAGS="-Wall -pedantic $CFLAGS"
	CXXFLAGS="-Wall -pedantic -Wno-long-long $CXXFLAGS"
fi

###########
# Options #
###########

# TODO provide correct switches for Sun C++ compiler too
if test "$GCC" = yes; then
	AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug],
		[compile with debugging support])])

	AS_IF([test "$enable_debug" = yes],
		[CFLAGS="-ggdb -O0 $CFLAGS"
	 	CXXFLAGS="-ggdb -O0 $CXXFLAGS"
	 	AC_DEFINE(DEBUG, 1, [Define if debugging is enabled.])],
		[CFLAGS="-O2 $CFLAGS"
	 	CXXFLAGS="-O2 $CXXFLAGS"])
fi
if test "$ac_cv_cxx_compiler_suncc" = yes; then
	CXXFLAGS="-library=stdcxx4 $CXXFLAGS"
fi
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
		 po/Makefile.in
		 src/Makefile
		 tests/Makefile])
AC_OUTPUT
