dnl Autotools config.m4 for PHP extension norce

dnl Comments in this file start with the string 'dnl' (discard to next line).
dnl Remove where necessary.

dnl If extension references and depends on an external library package, use
dnl the '--with-norce' configure option:
dnl PHP_ARG_WITH([norce],
dnl   [for norce support],
dnl   [AS_HELP_STRING([--with-norce],
dnl     [Include norce support])])

dnl Otherwise use the '--enable-norce' configure option:
PHP_ARG_ENABLE([norce],
  [whether to enable norce support],
  [AS_HELP_STRING([--enable-norce],
    [Enable norce support])],
  [no])

AS_VAR_IF([PHP_NORCE], [no],, [
  dnl This section is executed when extension is enabled with one of the above
  dnl configure options. Adjust and add tests here.

  dnl
  dnl Use and adjust this code block if extension depends on external library
  dnl package which supports pkg-config.
  dnl
  dnl Find library package with pkg-config.
  dnl PKG_CHECK_MODULES([LIBFOO], [foo])
  dnl
  dnl Or if you need to check for a particular library version with pkg-config,
  dnl you can use comparison operators. For example:
  dnl PKG_CHECK_MODULES([LIBFOO], [foo >= 1.2.3])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo < 3.4])
  dnl PKG_CHECK_MODULES([LIBFOO], [foo = 1.2.3])
  dnl
  dnl Add library compilation and linker flags to extension.
  dnl PHP_EVAL_INCLINE([$LIBFOO_CFLAGS])
  dnl PHP_EVAL_LIBLINE([$LIBFOO_LIBS], [NORCE_SHARED_LIBADD])
  dnl
  dnl Check for library and symbol presence.
  dnl LIBNAME=norce # you may want to change this
  dnl LIBSYMBOL=norce # you most likely want to change this
  dnl
  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are using pkg-config:
  dnl PHP_CHECK_LIBRARY([$LIBNAME], [$LIBSYMBOL],
  dnl   [AC_DEFINE([HAVE_NORCE_FEATURE], [1],
  dnl     [Define to 1 if norce has the 'FEATURE'.])],
  dnl   [AC_MSG_FAILURE([FEATURE not supported by your norce library.])],
  dnl   [$LIBFOO_LIBS])
  dnl

  dnl
  dnl Or use and adjust this code block if extension depends on external library
  dnl package, which does not support pkg-config.
  dnl
  dnl Path to library package can be given as parameter (--with-norce=<DIR>)
  dnl SEARCH_PATH="/usr/local /usr" # you might want to change this
  dnl SEARCH_FOR="/include/norce.h" # you most likely want to change this
  dnl AS_IF([test -r $PHP_NORCE/$SEARCH_FOR],
  dnl   [NORCE_DIR=$PHP_NORCE],
  dnl   [
  dnl     for i in $SEARCH_PATH; do
  dnl       AS_IF([test -r $i/$SEARCH_FOR],
  dnl         [NORCE_DIR=$i; break;])
  dnl     done
  dnl   ])
  dnl
  dnl AC_MSG_CHECKING([for norce library package])
  dnl AS_VAR_IF([NORCE_DIR],, [
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the norce library package])
  dnl ], [AC_MSG_RESULT([found in $NORCE_DIR])])
  dnl
  dnl Add include flag where library package headers are located on the system.
  dnl PHP_ADD_INCLUDE([$NORCE_DIR/include])
  dnl
  dnl Check for library and symbol presence.
  dnl LIBNAME=norce # you may want to change this
  dnl LIBSYMBOL=norce # you most likely want to change this
  dnl
  dnl If you need to check for a particular library function (e.g. a conditional
  dnl or version-dependent feature) and you are not using pkg-config:
  dnl PHP_CHECK_LIBRARY([$LIBNAME], [$LIBSYMBOL], [
  dnl     PHP_ADD_LIBRARY_WITH_PATH([$LIBNAME],
  dnl       [$NORCE_DIR/$PHP_LIBDIR],
  dnl       [NORCE_SHARED_LIBADD])
  dnl     AC_DEFINE([HAVE_NORCE_FEATURE], [1],
  dnl       [Define to 1 if norce has the 'FEATURE'.])
  dnl   ],
  dnl   [AC_MSG_FAILURE([FEATURE not supported by your norce library.])],
  dnl   [-L$NORCE_DIR/$PHP_LIBDIR -lm])
  dnl

  dnl Add linked libraries flags for shared extension to the generated Makefile.
  dnl PHP_SUBST([NORCE_SHARED_LIBADD])

  dnl Define a preprocessor macro to indicate that this PHP extension can
  dnl be dynamically loaded as a shared module or is statically built into PHP.
  AC_DEFINE([HAVE_NORCE], [1],
    [Define to 1 if the PHP extension 'norce' is available.])

  dnl Configure extension sources and compilation flags.
  PHP_NEW_EXTENSION([norce],
    [norce.c],
    [$ext_shared],,
    [-DZEND_ENABLE_STATIC_TSRMLS_CACHE=1])
])
