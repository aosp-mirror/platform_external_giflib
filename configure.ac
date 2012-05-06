dnl Process this file with autoconf to produce a configure script.
AC_INIT(giflib, [4.2.0], [esr@thyrsus.com], giflib)
AC_CONFIG_SRCDIR([lib/dgif_lib.c])
AM_INIT_AUTOMAKE([gnu dist-bzip2 -Wall])

dnl Note: config.h is not used in the current build
dnl We leave this in place only to supptess an error message at autogen time
AM_CONFIG_HEADER(config.h)

dnl Checks for programs.
AC_PROG_LIBTOOL
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AM_PROG_CC_C_O

dnl Shared-library version
SHLIBVERSION=5:7:1

dnl Define warnings if we're compiling with gcc.
dnl Set it in CFLAGS so the end-user can disable it if they wish.
if test "x$GCC" = "xyes" ; then
  CFLAGS="$CFLAGS -Wall"
fi

dnl Checks for libraries.

AC_CHECK_LIB(m, pow, [MATH_LIB="${MATH_LIB} -lm"
		      COMPILABLE_EXTRAS="${COMPILABLE_EXTRAS} gifclrmp${EXEEXT} gifrotat${EXEEXT}"] )

AC_ARG_ENABLE([x11], AC_HELP_STRING([--enable-x11],[support conversion directly to X11 windows[[default=detect]]]),
    enable_x11=$enableval, enable_x11=detect)

if test "x$enable_x11" != "xno" ; then
  AC_PATH_XTRA
  AC_CHECK_HEADERS([X11/Xlib.h X11/Xutil.h],
    AC_CHECK_LIB(X11, main, [X11_LIB="${X11_LIB} ${X_PRE_LIBS} $X_LIBS ${X_EXTRA_LIBS} -lX11"
                 AC_DEFINE(HAVE_LIBX11, [1], [Define if the X11 library is installed])
                 COMPILABLE_EXTRAS="${COMPILABLE_EXTRAS} gif2x11${EXEEXT}"],, $X_LIBS))
fi

DEVS="${X11_LIB}" 
AC_SUBST(DEVS)
AC_SUBST(COMPILABLE_EXTRAS)
AC_SUBST(X11_LIB)
AC_SUBST(X_CFLAGS)
AC_SUBST(MATH_LIB)
AC_SUBST(SHLIBVERSION)

AC_CHECK_HEADERS(limits.h, ,
                 AC_MSG_ERROR([giflib needs limits.h to compile]))
AC_CHECK_HEADERS(fcntl.h, ,
		 AC_MSG_ERROR([giflib needs fcntl.h to compile]))
AC_CHECK_HEADERS(stdint.h, ,
		 AC_MSG_ERROR([giflib needs stdint.h to compile]))
AC_CHECK_HEADERS(stdarg.h, ,
		 AC_MSG_ERROR([giflib needs stdarg.h to compile]))

AC_CONFIG_FILES([
util/Makefile
lib/Makefile
Makefile
doc/Makefile
pic/Makefile
])
AC_OUTPUT
