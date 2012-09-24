dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

AC_INIT(bijiben, 0.1)
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

dnl _FOR_DOC
AC_CONFIG_MACRO_DIR([m4])
dnl end of _FOR_DOC

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_PROG_CC

GNOME_COMMON_INIT
GNOME_DOC_PREPARE


GNOME_DOC_INIT([0.17.3])


dnl ***************************************************************************
dnl Internatinalization
dnl ***************************************************************************
GETTEXT_PACKAGE=bijiben
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])



AM_PROG_LIBTOOL


dnl tracker-sparql might be 0.12

PKG_CHECK_MODULES(BIJIBEN, tracker-sparql-0.14 clutter-gtk-1.0 
[gtk+-3.0 >= 3.5.19 libxml-2.0 glib-2.0 zeitgeist-1.0 glib-2.0])

GLIB_GSETTINGS

LIBGD_INIT([
  gtk-hacks
  main-view
  main-toolbar
])

AC_CONFIG_FILES([
Makefile
libgd/Makefile
data/Makefile
data/bijiben.desktop.in
data/icons/Makefile
help/Makefile
po/Makefile.in
src/libbiji/Makefile
src/Makefile
])

AC_OUTPUT

echo "	Version:	$VERSION

	Run make to compile."


