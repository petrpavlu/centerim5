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

# Hurray for <http://www-src.lip6.fr/homepages/Alexandre.Duret-Lutz/autotools.html>
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

#########################
# Checks for libraries. #
#########################

# libpurple
PKG_CHECK_MODULES(PURPLE, [purple >= 0.21], , [
        AC_MSG_RESULT(no)
	AC_MSG_ERROR([

You must have the libpurple development headers installed to build.
])])
AC_SUBST(PURPLE_CFLAGS)
AC_SUBST(PURPLE_LIBS)

# We use glibmm so no need to check all these libraries
# PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.0.0 gobject-2.0 gmodule-2.0 gthread-2.0], , [
#         AC_MSG_RESULT(no)
# 	AC_MSG_ERROR([
#
# You must have the GLib 2.0 development headers installed to build.
# ])])
# AC_SUBST(GLIB_CFLAGS)
# AC_SUBST(GLIB_LIBS)

# glibmm
PKG_CHECK_MODULES(GLIBMM, [glibmm-2.4 >= 2.12.0], , [
        AC_MSG_RESULT(no)
	AC_MSG_ERROR([

You must have the glibmm-2.4 development headers installed to build.
])])
AC_SUBST(GLIBMM_CFLAGS)
AC_SUBST(GLIBMM_LIBS)

# TODO -- verify which should be the latest ncurses library supported and check it too
AC_PATH_LIB_LIBCONFIG
AC_PATH_LIB(ncursesw, , ncurses.h, ncursesw5-config, , , [
    AC_MSG_RESULT(no)
    AC_ERROR(You must have the ncursesw library and developement headers installed to build.)
    ])

# we need -lpanel along with ncursesw (maybe need to check it too  ? )
NCURSESW_LIBS="$NCURSESW_LIBS -lpanel"

AC_SUBST(NCURSESW_LIBS)
AC_SUBST(NCURSESW_CFLAGS)

############################
# Checks for header files. #
############################

##################################################
# check whether make supports order-only targets #
##################################################
# TODO this check does not seem to work correctly
# it was taken from centerim 4's mob branch

AC_CACHE_CHECK([whether ${MAKE-make} supports order-only targets],
[centerim_cv_make_order_only],
[cat >conftest.make <<\_ACEOF
SHELL = /bin/sh
all: | say

say:
       echo say

.PHONY: all say
_ACEOF
centerim_cv_make_order_only=no

${MAKE-make} -f conftest.make >/dev/null 2>&1 && centerim_cv_make_order_only=yes
rm -f conftest.make
])
if test x$centerim_cv_make_order_only = xyes; then
  ORDER_ONLY="|"
fi

##################################################################
# Checks for typedefs, structures, and compiler characteristics. #
##################################################################
AC_C_CONST

AC_SUBST(ORDER_ONLY)

#################################
# Checks for library functions. #
#################################

###########
# Options #
###########

AC_ARG_ENABLE(debug, [AC_HELP_STRING([--enable-debug],
        [compile with debugging support])], , enable_debug=no)

if test "x$enable_debug" = "xyes" ; then
        AC_DEFINE(DEBUG, 1, [Define if debugging is enabled.])
fi

########################
# Create output files. #
########################
AC_CONFIG_FILES([Makefile po/Makefile.in
		 cppconsui/Makefile
		 src/Makefile])
AC_OUTPUT
