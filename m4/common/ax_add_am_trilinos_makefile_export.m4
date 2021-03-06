# ===========================================================================
#   http://autoconf-archive.cryp.to/ax_add_am_trilinos_makefile_export.html
# ===========================================================================
#
# SYNOPSIS
#
#   Using the AX_AM_MACROS framework, add a Trilinos Makefile.export
#   include to every file containing @INC_AMINCLUDE@.
#
#   AX_ADD_AM_TRILINOS_MAKEFILE_EXPORT(EXPORT_SUFFIX [, ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Checks if a file named <Makefile.export.EXPORT_SUFFIX> appears in
#   the $TRILINOS_INCLUDE directory.  If so, adds an include for it
#   using the AX_AM_MACROS framework.
#
#   If ACTION-IF-NOT-FOUND is not provided, configure fails.
#
# LAST MODIFICATION
#
#   2008-08-12
#
# COPYLEFT
#
#   Copyright (c) 2008 Rhys Ulerich <rhys.ulerich@gmail.com>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.

AC_DEFUN([AX_ADD_AM_TRILINOS_MAKEFILE_EXPORT],[
AC_REQUIRE([AX_TRILINOS_BASE])
AC_REQUIRE([AX_AM_MACROS])

AC_CACHE_CHECK(
    [for file ${TRILINOS_INCLUDE}/Makefile.export.$1], 
    [ax_cv_add_am_trilinos_makefile_export_]translit($1,[. ],[_])[_exists],
    [[ax_cv_add_am_trilinos_makefile_export_]translit($1,[. ],[_])[_exists]=no
     test -f "${TRILINOS_INCLUDE}/Makefile.export.$1" && dnl
     [ax_cv_add_am_trilinos_makefile_export_]translit($1,[. ],[_])[_exists]=yes])

if test "${[ax_cv_add_am_trilinos_makefile_export_]translit($1,[. ],[_])[_exists]}" = "yes"
then
  AX_ADD_AM_MACRO([
  include ${TRILINOS_INCLUDE}/Makefile.export.$1
  ])
  if test "$1" = "Epetra"; then
    ax_epetra=yes
  else
    ax_trilinos=yes
  fi
else
    #Kemelli- TODO: is this part of the conditional statement ever called?
    ifelse([$2],,AC_MSG_ERROR([Could not find ${TRILINOS_INCLUDE}/Makefile.export.$1. Was Trilinos compiled with: -D Trilinos_ENABLE_$1:BOOL=ON??]),[
    AC_MSG_WARN([Could not find ${TRILINOS_INCLUDE}/Makefile.export.$1. Was Trilinos compiled with: -D Trilinos_ENABLE_$1:BOOL=ON??])
    $2])
fi

])
