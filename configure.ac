dnl Process this file with autoconf to produce a configure script.
AC_INIT(configure.ac)
AC_CONFIG_HEADERS(config.h)
AC_CONFIG_FILES([Makefile include/Makefile include/cddb/Makefile \
                 lib/Makefile examples/Makefile tests/Makefile])
AM_INIT_AUTOMAKE(libcddb, 0.9.0)
AM_MAINTAINER_MODE

dnl Process custom command-line options
AC_ARG_ENABLE(debug,
              [  --enable-debug          enable debug mode [default=no]],
              enable_debug=yes, enable_debug=no)
if test x$enable_debug = xyes; then
    AC_DEFINE([DEBUG],1,[Define to enable debug output])
fi

dnl Check for target
AC_CANONICAL_HOST
case $host in
    *beos*)
        AC_DEFINE([BEOS],1,[Define if compiling on BeOS system.])
        ;;
esac

dnl Checks for programs.
AC_PROG_AWK
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

dnl Checks for libraries.
AM_PATH_CHECK([], enable_check=yes, enable_check=no)
AM_CONDITIONAL(REGRESSION_TESTS, test "x$enable_check" = "xyes")

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(unistd.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_TYPE_SIZE_T

dnl Checks for library functions.
AC_FUNC_ALLOCA
AC_FUNC_VPRINTF
AC_CHECK_FUNCS(mkdir regcomp socket strdup strtol)

dnl Check and add some GCC specific warning flags
dnl - we do this as the last thing so that a possible -Werror flag
dnl - does not cause a failure in one of the other tests above
if test "x$GCC" != "xyes" 
then
    echo "*** non GNU CC compiler detected."
    echo "*** This package has not been tested very well with non GNU compilers"
    echo "*** you should try to use 'gcc' for compiling this package."
else
    WARN_CFLAGS="-Wall -Wchar-subscripts -Wmissing-prototypes -Wmissing-declarations"
    WARN_CFLAGS="$WARN_CFLAGS -Wunused -Wpointer-arith -Wwrite-strings -Wnested-externs"
    WARN_CFLAGS="$WARN_CFLAGS -Wno-sign-compare"
    MAINTAINER_CFLAGS="-std=c99 -pedantic -Werror"

    if test "x$enable_maintainer_mode" = "xyes"; then
        maintainer_flags=""
        for MOPT in $MAINTAINER_CFLAGS; do
            SAVE_CFLAGS="$CFLAGS"
            CFLAGS="$CFLAGS $MOPT"
            AC_MSG_CHECKING([whether $CC accepts $MOPT])
            AC_TRY_COMPILE([], [], has_option=yes, has_option=no)
            CFLAGS="$SAVE_CFLAGS"
            AC_MSG_RESULT($has_option)
            if test "x$has_option" = "xyes"; then
                maintainer_flags="$maintainer_flags $MOPT"
            fi
            unset has_option
            unset SAVE_CFLAGS
        done
        unset MOPT
        MAINTAINER_CFLAGS="$maintainer_flags"
        unset maintainer_flags
    else
        MAINTAINER_CFLAGS=""
    fi

    warning_flags=""
    for WOPT in $WARN_CFLAGS; do
        SAVE_CFLAGS="$CFLAGS"
        CFLAGS="$CFLAGS $WOPT"
        AC_MSG_CHECKING([whether $CC accepts $WOPT])
        AC_TRY_COMPILE([], [], has_option=yes, has_option=no)
        CFLAGS="$SAVE_CFLAGS"
        AC_MSG_RESULT($has_option)
        if test "x$has_option" = "xyes"; then
            warning_flags="$warning_flags $WOPT"
        fi
        unset has_option
        unset SAVE_CFLAGS
    done
    unset WOPT
    WARN_CFLAGS="$warning_flags"
    unset warning_flags
fi
CFLAGS="$CFLAGS $WARN_CFLAGS $MAINTAINER_CFLAGS"

dnl Create makefiles
AC_OUTPUT
