AC_PREREQ(2.57)
AC_INIT(language-selector, 0.1, dottedmag@dottedmag.net)
AM_INIT_AUTOMAKE(foreign)

AC_LANG_C
AC_PROG_CC

PKG_CHECK_MODULES(ECORE,[ecore])
PKG_CHECK_MODULES(ECORE_FILE,[ecore-file])
PKG_CHECK_MODULES(ECORE_EVAS,[ecore-evas])
PKG_CHECK_MODULES(EVAS,[evas])
PKG_CHECK_MODULES(EDJE,[edje])
PKG_CHECK_MODULES(ECHOICEBOX,[echoicebox])
PKG_CHECK_MODULES(LIBLANGUAGE,[language])

AC_OUTPUT(Makefile src/Makefile themes/Makefile)
