dnl Find the compiler and linker flags for Cyrus SASL.
dnl
dnl Finds the compiler and linker flags for linking with v2 of the Cyrus SASL
dnl library.  Provides the --with-sasl, --with-sasl-lib, and
dnl --with-sasl-include configure options to specify non-standard paths to the
dnl Cyrus SASL library.
dnl
dnl Provides the macro RRA_LIB_SASL and sets the substitution variables
dnl SASL_CPPFLAGS, SASL_LDFLAGS, and SASL_LIBS.  Also provides
dnl RRA_LIB_SASL_SWITCH to set CPPFLAGS, LDFLAGS, and LIBS to include the
dnl Cyrus SASL v2 library, saving the current values first, and
dnl RRA_LIB_SASL_RESTORE to restore those settings to before the last
dnl RRA_LIB_SASL_SWITCH.  Defines HAVE_SASL and sets rra_use_SASL to true.
dnl
dnl Provides the RRA_LIB_SASL_OPTIONAL macro, which should be used if Cyrus
dnl SASL support is optional.  This macro will still always set the
dnl substitution variables, but they'll be empty if the Cyrus SASL library was
dnl not found or if --without-sasl was given.  Defines HAVE_SASL and sets
dnl rra_use_SASL to true if the Cyrus SASL library is found and --without-sasl
dnl is not given.
dnl
dnl Depends on the lib-helper.m4 framework.
dnl
dnl The canonical version of this file is maintained in the rra-c-util
dnl package, available at <https://www.eyrie.org/~eagle/software/rra-c-util/>.
dnl
dnl Written by Russ Allbery <eagle@eyrie.org>
dnl Copyright 2018, 2022 Russ Allbery <eagle@eyrie.org>
dnl Copyright 2013
dnl     The Board of Trustees of the Leland Stanford Junior University
dnl
dnl This file is free software; the authors give unlimited permission to copy
dnl and/or distribute it, with or without modifications, as long as this
dnl notice is preserved.
dnl
dnl SPDX-License-Identifier: FSFULLR

dnl Save the current CPPFLAGS, LDFLAGS, and LIBS settings and switch to
dnl versions that include the Cyrus SASL flags.  Used as a wrapper, with
dnl RRA_LIB_SASL_RESTORE, around tests.
AC_DEFUN([RRA_LIB_SASL_SWITCH], [RRA_LIB_HELPER_SWITCH([SASL])])

dnl Restore CPPFLAGS, LDFLAGS, and LIBS to their previous values (before
dnl RRA_LIB_SASL_SWITCH was called).
AC_DEFUN([RRA_LIB_SASL_RESTORE], [RRA_LIB_HELPER_RESTORE([SASL])])

dnl Checks if the Cyrus SASL library is present.  The single argument, if
dnl "true", says to fail if the Cyrus SASL library could not be found.
AC_DEFUN([_RRA_LIB_SASL_INTERNAL],
[RRA_LIB_HELPER_PATHS([SASL])
 RRA_LIB_SASL_SWITCH
 AC_CHECK_HEADER([sasl/sasl.h],
    [AC_CHECK_LIB([sasl2], [sasl_getprop],
        [SASL_LIBS="-lsasl2"],
        [AS_IF([test x"$1" = xtrue],
            [AC_MSG_ERROR([cannot find usable Cyrus SASL library])])])],
    [AS_IF([test x"$1" = xtrue],
        [AC_MSG_ERROR([cannot find usable Cyrus SASL header])])])
 RRA_LIB_SASL_RESTORE])

dnl The main macro for packages with mandatory Cyrus SASL support.
AC_DEFUN([RRA_LIB_SASL],
[RRA_LIB_HELPER_VAR_INIT([SASL])
 RRA_LIB_HELPER_WITH([sasl], [Cyrus SASL], [SASL])
 _RRA_LIB_SASL_INTERNAL([true])
 rra_use_SASL=true
 AC_DEFINE([HAVE_SASL], 1, [Define if libsasl2 is available.])])

dnl The main macro for packages with optional Cyrus SASL support.
AC_DEFUN([RRA_LIB_SASL_OPTIONAL],
[RRA_LIB_HELPER_VAR_INIT([SASL])
 RRA_LIB_HELPER_WITH_OPTIONAL([sasl], [Cyrus SASL], [SASL])
 AS_IF([test x"$rra_use_SASL" != xfalse],
    [AS_IF([test x"$rra_use_SASL" = xtrue],
        [_RRA_LIB_SASL_INTERNAL([true])],
        [_RRA_LIB_SASL_INTERNAL([false])])])
 AS_IF([test x"$SASL_LIBS" = x],
    [RRA_LIB_HELPER_VAR_CLEAR([SASL])],
    [rra_use_SASL=true
     AC_DEFINE([HAVE_SASL], 1, [Define if libsasl2 is available.])])])
