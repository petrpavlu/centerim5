# *- Autoconf -*-

# hurray for <http://www.lrde.epita.fr/~adl/autotools.html>

AC_PREREQ([2.59])
AC_INIT([centerim5],
	m4_esyscmd([misc/git-version-gen .tarball-version]),
	[http://bugzilla.centerim.org/], [centerim5],
	[http://www.centerim.org/])
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

LT_INIT([disable-static])

#########################
# Checks for libraries. #
#########################

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.17])

# libpurple
# v2.7.0 is needed because of purple_account_get_name_for_display(), this
# version was released on 2010-05-13 and is available in both Debian Stable
# (squeeze) and CentOS 6
# find . \( -name \*.cpp -o -name \*.h \) -print0 | xargs -0 sed -n 's/.*\(purple_[^( ]*\)(.*/\1/p' | sort | uniq | less
PKG_CHECK_MODULES([PURPLE], [purple >= 2.7.0])
AC_SUBST([PURPLE_CFLAGS])
AC_SUBST([PURPLE_LIBS])

# glib
# v2.16.0 is needed because of g_markup_parse_context_get_element_stack(),
# this version was released on 2009-03-13
# find . \( -name \*.cpp -o -name \*.h \) -print0 | xargs -0 sed -n 's/.*\(g_[^ (]*\)(.*/\1/p' | sort | uniq | less
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.16.0])
AC_SUBST([GLIB_CFLAGS])
AC_SUBST([GLIB_LIBS])

# extaction plugin requires a newer version of glib, check if it's available
PKG_CHECK_EXISTS([glib-2.0 >= 2.32.0], [build_extaction=yes],
	[build_extaction=no
	AC_MSG_WARN([cannot find glib version >= 2.32, extaction plugin disabled])])
AM_CONDITIONAL(BUILD_EXTACTION, test "x$build_extaction" = xyes)

# libsigc++
# v2.2.0 was released on 2008-02-22
PKG_CHECK_MODULES([SIGC], [sigc++-2.0 >= 2.2.0])
AC_SUBST([SIGC_CFLAGS])
AC_SUBST([SIGC_LIBS])

# ncursesw
AC_PATH_LIB_LIBCONFIG
AC_PATH_LIB([ncursesw], , [ncurses.h], [ncursesw5-config], , , [
	AC_MSG_RESULT([no])
	AC_ERROR([You must have the ncursesw library and developement headers installed to build.])])

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

AC_ARG_ENABLE([strict], [AC_HELP_STRING([--enable-strict],
	[enable extra compiler warnings during build])])

if test "$GCC" = yes; then
	AS_IF([test "$enable_strict" = yes],
		[CFLAGS="-Wall -Wextra -pedantic $CFLAGS"
		CXXFLAGS="-Wall -Wextra -pedantic -Wno-long-long $CXXFLAGS"])
fi

###########
# Options #
###########

AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug],
	[compile with debugging support])])

# TODO provide correct switches for other compilers too
if test "$GCC" = yes; then
	AS_IF([test "$enable_debug" = yes],
		[CFLAGS="-ggdb -O0 $CFLAGS"
		CXXFLAGS="-ggdb -O0 $CXXFLAGS"],
		[CFLAGS="-O2 $CFLAGS"
		CXXFLAGS="-O2 $CXXFLAGS"])
fi
AC_SUBST([CFLAGS])
AC_SUBST([CXXFLAGS])

AS_IF([test "$enable_debug" = yes],
	[AC_DEFINE([DEBUG], [1], [Define if debugging is enabled.])])

# adl_RECURSIVE_EVAL(VALUE, RESULT)
# =================================
# Interpolate the VALUE in loop until it doesn't change,
# and set the result to $RESULT.
# WARNING: It's easy to get an infinite loop with some unsane input.
AC_DEFUN([adl_RECURSIVE_EVAL],
[_lcl_receval="$1"
$2=`(test "$prefix" = NONE && prefix="$ac_default_prefix"
	test "$exec_prefix" = NONE && exec_prefix="$prefix"
	_lcl_receval_old=''
	while test "[$]_lcl_receval_old" != "[$]_lcl_receval"; do
		_lcl_receval_old="[$]_lcl_receval"
		eval _lcl_receval="\"[$]_lcl_receval\""
	done
	echo "[$]_lcl_receval")`])

# Define locale and pkgdir directories. It would be nicer to do this in
# Makefile.ams by adding appropriate '-D' compiler options to respective
# CPPFLAGS, which is a common solution that can be seen in other projects. In
# that case, it isn't needed to resolve the paths at the configure time using
# the adl_RECURSIVE_EVAL function. However this solution unfortunately doesn't
# work correctly when values of the defines are changed, the source files that
# use these defines don't get correctly recompiled. To fix this, the defines
# have to be put into the config.h file.
adl_RECURSIVE_EVAL([$localedir], [localedir_eval])
AC_DEFINE_UNQUOTED([LOCALEDIR], ["$localedir_eval"],
	[Define to the locale directory.])

adl_RECURSIVE_EVAL([$pkglibdir], [pkglibdir_eval])
AC_DEFINE_UNQUOTED([PKGLIBDIR], ["$pkglibdir_eval"],
	[Define to the pkglib directory.])

########################
# Create output files. #
########################
AC_CONFIG_FILES([Makefile
		 contrib/Makefile
		 cppconsui/Makefile
		 doc/Makefile
		 doc/doxygen-cppconsui.conf
		 misc/Makefile
		 plugins/Makefile
		 po/Makefile.in
		 src/Makefile
		 tests/Makefile])
AC_OUTPUT
