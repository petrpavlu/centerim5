#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([centerim], [4.22.0], [http://sourceforge.net/projects/centerim])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE([-Wall])

AC_GNU_SOURCE

########################
# Checks for programs. #
########################
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.16.1])

#########################
# Checks for libraries. #
#########################

# glib
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.0.0 gobject-2.0 gmodule-2.0 gthread-2.0], , [
        AC_MSG_RESULT(no)
	AC_MSG_ERROR([

You must have the GLib 2.0 development headers installed to build.
])])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

# libpurple
PKG_CHECK_MODULES(PURPLE, [purple >= 0.21], , [
        AC_MSG_RESULT(no)
	AC_MSG_ERROR([

You must have the libpurple development headers installed to build.
])])
AC_SUBST(PURPLE_CFLAGS)
AC_SUBST(PURPLE_LIBS)

# glibmm
PKG_CHECK_MODULES(GLIBMM, [glibmm-2.4 >= 2.12.8], , [
        AC_MSG_RESULT(no)
	AC_MSG_ERROR([

You must have the glibmm-2.4 development headers installed to build.
])])
AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)

# curses
AC_CHECK_CURSES
AC_CHECK_CURSES
if test "$has_curses" = false ; then
        AC_ERROR(You must have the curses development headers installed to build.)
fi
#TODO remove this dirty hack!
CURSES_LIBS="-lpanel -lcursesw"
AC_SUBST(CURSES_LIBS)
AC_SUBST(CURSES_INCLUDEDIR)

############################
# Checks for header files. #
############################

##################################################################
# Checks for typedefs, structures, and compiler characteristics. #
##################################################################
AC_C_CONST

#################################
# Checks for library functions. #
#################################

########################
# Create output files. #
########################
AC_CONFIG_FILES([Makefile po/Makefile.in
		 cppconsui/Makefile
		 src/Makefile])
AC_OUTPUT
