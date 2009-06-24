#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.59)
AC_INIT([centerim], [5.0-alpha1], [http://sourceforge.net/projects/centerim])
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
AM_GNU_GETTEXT_VERSION([0.16.1])

#########################
# Checks for libraries. #
#########################

# libpurple
#PKG_CHECK_MODULES(PURPLE, [purple >= 0.21], , [
#        AC_MSG_RESULT(no)
#	AC_MSG_ERROR([
#
#You must have the libpurple development headers installed to build.
#])])

AC_ARG_WITH(libpurple,
	AC_HELP_STRING([--with-libpurple=DIR],[look for the purple library in DIR]),
	[with_libpurple=$withval],[with_libpurple="no"])

if test "x$with_libpurple" != "xno" ; then

        AC_MSG_CHECKING(for libpurple)
        if test -z "$with_libpurple" -o "x$with_libpurple" = "xyes"; then
                for ac_dir in $with_libpurple /usr/local /usr; do
                        if test -f "$ac_dir/include/libpurple/account.h"; then
                                with_libpurple=$ac_dir
                                break;
                        fi
                done
        fi

        if test -n "$with_libpurple" -a "x$with_libpurple" != "xno"; then
                AC_MSG_RESULT([found in $with_libpurple])
                PURPLE_CFLAGS="-I${with_libpurple}include"
		PURPLE_LIBS="-L${with_libpurple}lib -lpurple"
		PURPLE_DATADIR="${with_libpurple}share"
		PURPLE_LIBDIR="${with_libpurple}lib"
		ac_configure_args="$ac_configure_args  --with-libpurple=$with_libpurple"
        else
		AC_MSG_RESULT(no)
		echo No libpurple found in $with_libpurple, /usr/local or /usr;
        fi

else
	AM_PATH_PURPLE([2.4.0])
fi

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
#NCURSESW_LIBS="$NCURSESW_LIBS -lpanel"

#AC_SUBST(NCURSESW_LIBS)
#AC_SUBST(NCURSESW_CFLAGS)

CURSES_LIBS=$NCURSESW_LIBS
CURSES_INCLUDEDIR=$NCURSESW_CFLAGS
has_curses=true
AC_DEFINE(HAS_CURSES, 1, [Do we have curses?])

AC_SUBST(CURSES_LIBS)
AC_SUBST(CURSES_INCLUDEDIR)

#AC_CHECK_CURSES
AM_CONDITIONAL(HAS_CURSES, test "$has_curses" = true)

############################
# Checks for header files. #
############################

##################################################
# check whether make supports order-only targets #
##################################################

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

if test "x$GCC" = "xyes"; then
	dnl We enable -Wall later.
	dnl If it's set after the warning CFLAGS in the compiler invocation, it counteracts the -Wno... flags.
	dnl This leads to warnings we don't want.
	CFLAGS=`echo $CFLAGS |sed 's/-Wall//'`

	dnl ENABLE WARNINGS SUPPORTED BY THE VERSION OF GCC IN USE
	dnl
	dnl Future Possibilities
	dnl
	dnl Consider adding -Wbad-function-cast.
	dnl	This leads to spurious warnings using GPOINTER_TO_INT(), et al. directly on a function call.
	dnl		We'd need an intermediate variable.
	dnl
	dnl Consider adding -Wfloat-equal.
	dnl	This leads to warnings with Perl.
	dnl		Perhaps we could write ugly configure magic and pass -Wno-float-equal down to that subdirectory.
	dnl		On the other hand, it's probably actually broken, so maybe the Perl folks should fix that?
	dnl
	dnl Consider removing -Wno-sign-compare (from the -Wextra set) and fixing all those cases.
	dnl	This is likely non-trivial.
	dnl
	for newflag in \
		dnl	"-Wshadow" \
		dnl	"-Waggregate-return" \
			"-Wcast-align" \
		dnl	"-Wdeclaration-after-statement" \
			"-Wendif-labels" \
		dnl	"-Werror-implicit-function-declaration" \
			"-Wextra -Wno-sign-compare -Wno-unused-parameter" \
			"-Wformat-security" \
				"-Werror=format-security" \
			"-Winit-self" \
			"-Wmissing-declarations" \
			"-Wmissing-noreturn" \
		dnl	"-Wmissing-prototypes" \
			"-Wpointer-arith" \
			"-Wundef" \
	; do
		orig_CFLAGS="$CFLAGS"
		CFLAGS="$CFLAGS $newflag"
		AC_MSG_CHECKING(for $newflag option to gcc)
		AC_TRY_COMPILE([], [
			int main() {return 0;}
		], [
			AC_MSG_RESULT(yes)
			CFLAGS="$orig_CFLAGS"
			DEBUG_CFLAGS="$DEBUG_CFLAGS $newflag"
		], [
			AC_MSG_RESULT(no)
			CFLAGS="$orig_CFLAGS"
		])
	done

	if test "x$enable_fortify" = "xyes"; then
		AC_MSG_CHECKING(for FORTIFY_SOURCE support)
		AC_TRY_COMPILE([#include <features.h>], [
			int main() {
			#if !(__GNUC_PREREQ (4, 1) \
				|| (defined __GNUC_RH_RELEASE__ && __GNUC_PREREQ (4, 0)) \
				|| (defined __GNUC_RH_RELEASE__ && __GNUC_PREREQ (3, 4) \
					&& __GNUC_MINOR__ == 4 \
					&& (__GNUC_PATCHLEVEL__ > 2 \
						|| (__GNUC_PATCHLEVEL__ == 2 && __GNUC_RH_RELEASE__ >= 8))))
			#error No FORTIFY_SOURCE support
			#endif
				return 0;
			}
		], [
			AC_MSG_RESULT(yes)
			DEBUG_CFLAGS="$DEBUG_CFLAGS -Wp,-D_FORTIFY_SOURCE=2"
		], [
			AC_MSG_RESULT(no)
		])
	fi

	DEBUG_CFLAGS="-Wall $DEBUG_CFLAGS"
	CFLAGS="-g $CFLAGS"
fi

AC_SUBST(CFLAGS)
AC_SUBST(DEBUG_CFLAGS)

########################
# Create output files. #
########################
AC_CONFIG_FILES([Makefile po/Makefile.in
		 cppconsui/Makefile
		 tests/Makefile
		 src/Makefile])
AC_OUTPUT
