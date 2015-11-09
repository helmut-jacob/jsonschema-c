#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([jsonschema-c], [0,1], [ahamed@ocedo.com])



# Checks for header files.
AC_CHECK_HEADERS(string.h regex.h stdio.h json.h linkhash.h)

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.
AC_CHECK_FUNCS([memset regcomp])

AC_CHECK_LIB([json-c],json_object_get_type, [],[
         echo "Json-c library not found."
         exit -1])


AM_PROG_LIBTOOL
AM_INIT_AUTOMAKE([0.1 foreign])
AC_PROG_CC
AC_PROG_RANLIB
AC_CONFIG_FILES([Makefile
                 tests/Makefile])
LT_INIT
AC_OUTPUT
