# Process this file with autoconf to produce a configure script.

AC_INIT([CD Audio FileSystem], [0.1.0], [ben-claw@mail.ru], [cdafs])

AM_INIT_AUTOMAKE([-Wall -Werror foreign])
PKG_PROG_PKG_CONFIG

AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES([FUSE], [fuse >= 2.6 fuse < 3.0],
	[AC_DEFINE([FUSE_USE_VERSION], [26], [Required libfuse version])],
	[AC_MSG_ERROR([Compatible version of libfuse not found])]
)

AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
	Makefile
	src/Makefile
])
AC_OUTPUT
