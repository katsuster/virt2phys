dnl
dnl Autoconf configuration file for virt2phys.
dnl

AC_PREREQ(2.61)

AC_INIT([v2p], [0.1], [no-define])
AC_ARG_PROGRAM()

AC_CONFIG_SRCDIR([src/v2p.c])
AC_CONFIG_HEADER([config.h])
AC_CONFIG_AUX_DIR([conf])
AC_CONFIG_MACRO_DIR([m4])

AM_INIT_AUTOMAKE([foreign])
LT_INIT()


dnl ----------
dnl Checks for programs.
dnl ----------
AC_PROG_CC()
AM_PROG_CC_C_O()
AC_PROG_INSTALL()
AC_PROG_LN_S()
AC_PROG_LIBTOOL()


dnl ----------
dnl Checks for common headers and libraries.
dnl ----------
AC_HEADER_STDC()


dnl ----------
dnl Checks for '--enable-ABCD' arguments.
dnl ----------

AC_ARG_ENABLE(debug,
	[AS_HELP_STRING([--enable-debug], [turn on debugging (default=no)])],
	[], [enable_debug=no])
AC_MSG_CHECKING(whether to enable debuging)
AC_MSG_RESULT($enable_debug)
AM_CONDITIONAL(ENABLE_DEBUG, [test x$enable_debug = xyes])
if test x$enable_debug = xyes; then
	AC_DEFINE(FA_ENABLE_DEBUG, 1,
		[Define to enable the debug functions])
	CPPFLAGS="$CPPFLAGS -g -O0"
else
	CPPFLAGS="$CPPFLAGS -O2"
fi

dnl ----------------------------------------


dnl ----------
dnl Checks for '--with-ABCD' arguments.
dnl ----------

if test x$prefix = xNONE; then
	# FIXME: undocumented variable...
	my_prefix=$ac_default_prefix
else
	my_prefix=$prefix
fi

if test x$lt_sysroot != x; then
	CPPFLAGS="$CPPFLAGS --sysroot=$lt_sysroot "
	LDFLAGS="$LDFLAGS --sysroot=$lt_sysroot "
fi


dnl ----------
dnl  Specify the Makefile
dnl ----------
AC_CONFIG_FILES([Makefile src/Makefile])

AC_OUTPUT()
