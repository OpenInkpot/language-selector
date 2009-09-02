AC_PREREQ(2.57)
AC_INIT(language-selector, 0.1, dottedmag@dottedmag.net)
AM_INIT_AUTOMAKE(foreign)

AC_LANG_C
AC_PROG_CC

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES(EFL,[ecore ecore-file ecore-evas evas edje])
PKG_CHECK_MODULES(OI,[libchoicebox liblanguage libeoi])

AC_OUTPUT(Makefile src/Makefile)
