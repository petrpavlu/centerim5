AC_PREREQ([2.59])
AC_INIT([centerim5],
	m4_esyscmd([misc/git-version-gen .tarball-version]),
	[http://bugzilla.centerim.org/], [centerim5],
	[http://www.centerim.org/])
# Prevent AC_PROG_CC, AC_PROG_CXX from initializing CFLAGS and CXXFLAGS.
: ${CFLAGS=""}
: ${CXXFLAGS=""}
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_AUX_DIR([config])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([-Wall subdir-objects])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_GNU_SOURCE

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CXX
AX_CXX_COMPILE_STDCXX_11([noext])
AM_PROG_AR

LT_INIT([disable-static])

# Set compiler flags.

AC_ARG_ENABLE([strict], [AC_HELP_STRING([--enable-strict],
	[enable extra compiler warnings during build])])
AC_ARG_ENABLE([debug], [AC_HELP_STRING([--enable-debug],
	[compile with debugging support])])

# If not --enable-debug then try to set -O2 as default.
AS_IF([test "$enable_debug" != yes],
	[save_cflags="$CFLAGS"]
	[CFLAGS="-O2"]
	[AC_MSG_CHECKING([whether CC supports -O2])]
	[AC_LANG_PUSH([C])]
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
		[AC_MSG_RESULT([yes])]
		[AM_CFLAGS="$AM_CFLAGS -O2"],
		[AC_MSG_RESULT([no])])]
	[AC_LANG_POP([C])]
	[CFLAGS="$save_cflags"]

	[save_cxxflags="$CXXFLAGS"]
	[CXXFLAGS="-O2"]
	[AC_MSG_CHECKING([whether CXX supports -O2])]
	[AC_LANG_PUSH([C++])]
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
		[AC_MSG_RESULT([yes])]
		[AM_CXXFLAGS="$AM_CXXFLAGS -O2"],
		[AC_MSG_RESULT([no])])]
	[AC_LANG_POP([C++])]
	[CXXFLAGS="$save_cxxflags"])

# If --enable-strict is specified then enable compilation warnings.
AS_IF([test "$enable_strict" = yes],
	[save_cflags="$CFLAGS"]
	[CFLAGS="-Wall -Wextra -pedantic"]
	[AC_MSG_CHECKING([whether CC supports -Wall -Wextra -pedantic])]
	[AC_LANG_PUSH([C])]
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])]
		[AC_MSG_ERROR([the C compiler does not recognize options -Wall -Wextra -pedantic that are needed by --enable-strict])])]
	[AC_LANG_POP([C])]
	[CFLAGS="$save_cflags"]
	[AM_CFLAGS="$AM_CFLAGS -Wall -Wextra -pedantic"]

	[save_cxxflags="$CXXFLAGS"]
	[CXXFLAGS="-Wall -Wextra -pedantic -Wno-long-long -Wsuggest-override"]
	[AC_MSG_CHECKING([whether CXX supports -Wall -Wextra -pedantic -Wno-long-long -Wsuggest-override])]
	[AC_LANG_PUSH([C++])]
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])]
		[AC_MSG_ERROR([the C++ compiler does not recognize options -Wall -Wextra -pedantic -Wno-long-long -Wsuggest-override that are needed by --enable-strict])])]
	[AC_LANG_POP([C++])]
	[CXXFLAGS="$save_cxxflags"]
	[AM_CXXFLAGS="$AM_CXXFLAGS -Wall -Wextra -pedantic -Wno-long-long -Wsuggest-override"])

# If --enable-debug is specified then compile with -g -O0.
AS_IF([test "$enable_debug" = yes],
	[save_cflags="$CFLAGS"]
	[CFLAGS="-g -O0"]
	[AC_MSG_CHECKING([whether CC supports -g -O0])]
	[AC_LANG_PUSH([C])]
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])]
		[AC_MSG_ERROR([the C compiler does not recognize options -g -O0 that are needed by --enable-debug])])]
	[AC_LANG_POP([C])]
	[CFLAGS="$save_cflags"]
	[AM_CFLAGS="$AM_CFLAGS -g -O0"]

	[save_cxxflags="$CXXFLAGS"]
	[CXXFLAGS="-g -O0"]
	[AC_MSG_CHECKING([whether CXX supports -g -O0])]
	[AC_LANG_PUSH([C++])]
	[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
		[AC_MSG_RESULT([yes])],
		[AC_MSG_RESULT([no])]
		[AC_MSG_ERROR([the C++ compiler does not recognize options -g -O0 that are needed by --enable-debug])])]
	[AC_LANG_POP([C++])]
	[CXXFLAGS="$save_cxxflags"]
	[AM_CXXFLAGS="$AM_CXXFLAGS -g -O0"])
AC_SUBST([AM_CFLAGS])
AC_SUBST([AM_CXXFLAGS])
AS_IF([test "$enable_debug" = yes],
	[AC_DEFINE([DEBUG], [1], [Define if debugging is enabled.])])

# Checks for libraries.

AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.18.2])

# libpurple
# v2.7.0 is needed because of purple_account_get_name_for_display(), this
# version was released on 2010-05-13 and is available in both Debian Stable
# (squeeze) and CentOS 6.
# find . \( -name \*.cpp -o -name \*.h \) -print0 | xargs -0 sed -n 's/.*\(purple_[^( ]*\)(.*/\1/p' | sort | uniq | less
PKG_CHECK_MODULES([PURPLE], [purple >= 2.7.0])
AC_SUBST([PURPLE_CFLAGS])
AC_SUBST([PURPLE_LIBS])

# glib
# v2.16.0 is needed because of g_markup_parse_context_get_element_stack(),
# this version was released on 2009-03-13.
# find . \( -name \*.cpp -o -name \*.h \) -print0 | xargs -0 sed -n 's/.*\(g_[^ (]*\)(.*/\1/p' | sort | uniq | less
PKG_CHECK_MODULES([GLIB], [glib-2.0 >= 2.16.0])
AC_SUBST([GLIB_CFLAGS])
AC_SUBST([GLIB_LIBS])

# The extaction plugin requires a newer version of glib, check whether it is
# available.
PKG_CHECK_EXISTS([glib-2.0 >= 2.32.0], [build_extaction=yes],
	[build_extaction=no
	AC_MSG_WARN([cannot find glib version >= 2.32, extaction plugin disabled])])
AM_CONDITIONAL(BUILD_EXTACTION, test "$build_extaction" = yes)

# libsigc++
# v2.2.0 was released on 2008-02-22.
PKG_CHECK_MODULES([SIGC], [sigc++-2.0 >= 2.2.0])
AC_SUBST([SIGC_CFLAGS])
AC_SUBST([SIGC_LIBS])

# ncursesw
AC_CHECK_PROGS(NCURSES_CONFIG, [ncursesw6-config ncursesw5-config])
AS_IF([test "$NCURSES_CONFIG" = ""],
	[AC_MSG_ERROR([the ncursesw6-config or ncursesw5-config helper script must be installed to build])])
AC_PATH_LIB_LIBCONFIG
AC_PATH_LIB([ncursesw], , [ncurses.h], ["$NCURSES_CONFIG"], , , [
	AC_MSG_RESULT([no])
	AC_MSG_ERROR([the ncursesw library and developement headers must be installed to build])])

# adl_RECURSIVE_EVAL(VALUE, RESULT)
# =================================
# Interpolate the VALUE in loop until it does not change, and set the result
# to $RESULT.
# WARNING: It is easy to get an infinite loop with some unsane input.
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
# that case, it is not needed to resolve the paths at the configure time using
# the adl_RECURSIVE_EVAL function. However this solution unfortunately does not
# work correctly when values of the defines are changed, the source files that
# use these defines do not get correctly recompiled. To fix this, the defines
# have to be put into the config.h file.
adl_RECURSIVE_EVAL([$localedir], [localedir_eval])
AC_DEFINE_UNQUOTED([LOCALEDIR], ["$localedir_eval"],
	[Define to the locale directory.])

adl_RECURSIVE_EVAL([$libdir/AC_PACKAGE_NAME], [pkglibdir_eval])
AC_DEFINE_UNQUOTED([PKGLIBDIR], ["$pkglibdir_eval"],
	[Define to the pkglib directory.])

# Create output files.
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
