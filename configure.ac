AC_INIT([iptvd], [0.2])
AC_CONFIG_AUX_DIR([ac.d])
AM_INIT_AUTOMAKE([-Wall -Werror])
#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.64])

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netinet/in.h stdlib.h string.h sys/socket.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_MALLOC
AC_CHECK_FUNCS([inet_ntoa socket strdup])

AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
