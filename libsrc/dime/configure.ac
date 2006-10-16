
m4_define([DIME_MAJOR], [0])
m4_define([DIME_MINOR], [9])
m4_define([DIME_MICRO], [1])
m4_define([DIME_BETA], [])
m4_define([DIME_VERSION_STRING], [DIME_MAJOR.DIME_MINOR.DIME_MICRO[]DIME_BETA])

m4_define([DIME_ABI_VERSION], [0])
m4_define([DIME_ABI_AGE], [0])
m4_define([DIME_ABI_REVISION], [0])

# **************************************************************************

AC_INIT([dime], [DIME_VERSION_STRING], [dime-support@coin3d.org])

AC_CONFIG_SRCDIR([include/dime/Base.h])
AC_CONFIG_AUX_DIR([cfg])

AC_CANONICAL_SYSTEM

AC_SUBST([DIME_MAJOR_VERSION], [DIME_MAJOR])
AC_SUBST([DIME_MINOR_VERSION], [DIME_MINOR])
AC_SUBST([DIME_MICRO_VERSION], [DIME_MICRO])
AC_SUBST([DIME_BETA_VERSION], [DIME_MICRO])
AC_SUBST([DIME_VERSION], [DIME_VERSION_STRING])

VERSION=$DIME_VERSION

AC_SUBST(DIME_INTERFACE_AGE, 0)
AC_SUBST(DIME_BINARY_AGE, 0)

SIM_AC_MSVC_DSP_ENABLE_OPTION

SIM_AC_SETUP_MSVCPP_IFELSE
if $BUILD_WITH_MSVC; then
  SIM_AC_SETUP_MSVCRT([
    CPPFLAGS="$CPPFLAGS $sim_ac_msvcrt_CPPFLAGS"
    CFLAGS="$CFLAGS $sim_ac_msvcrt_CFLAGS"
    CXXFLAGS="$CXXFLAGS $sim_ac_msvcrt_CXXFLAGS"
    LDFLAGS="$LDFLAGS $sim_ac_msvcrt_LDFLAGS"
    LIBS="$LIBS $sim_ac_msvcrt_LIBS"
    # sim_ac_msvcrt_LIBLIBS is set up at the end
    # sim_ac_msvcrt_LIBLDFLAGS is set up at the end
    AC_SUBST([DIME_MSVC_LIBC], [$sim_ac_msvcrt])
  ])
fi

dnl  Locate C++ compiler and set C++ as the default compiler to run
dnl  in tests.
AC_PROG_CC
AC_PROG_CPP
AC_PROG_CXX
AC_PROG_CXXCPP
AC_LANG([C++])

dnl *******************************************************************
dnl * Library version control.
dnl *******************************************************************

dnl  Making releases:
dnl    DIME_MICRO_VERSION += 1;
dnl    DIME_INTERFACE_AGE += 1;
dnl    DIME_BINARY_AGE += 1;
dnl  if any functions have been added, set DIME_INTERFACE_AGE to 0.
dnl  if backwards compatibility has been broken,
dnl  set DIME_BINARY_AGE _and_ DIME_INTERFACE_AGE to 0.
dnl 

dnl  libtool versioning
LT_RELEASE=$DIME_MAJOR_VERSION.$DIME_MINOR_VERSION
LT_CURRENT=`expr $DIME_MICRO_VERSION - $DIME_INTERFACE_AGE`
LT_REVISION=$DIME_INTERFACE_AGE
LT_AGE=`expr $DIME_BINARY_AGE - $DIME_INTERFACE_AGE`
AC_SUBST(LT_RELEASE)
AC_SUBST(LT_CURRENT)
AC_SUBST(LT_REVISION)
AC_SUBST(LT_AGE)

AC_SUBST([SUFFIX], [])

#  These are used for constructing the dime-config file.
AC_SUBST([DIME_EXTRA_CPPFLAGS], [$CPPFLAGS])
AC_SUBST([DIME_EXTRA_CFLAGS], [$CFLAGS])
AC_SUBST([DIME_EXTRA_CXXFLAGS], [$CXXFLAGS])
AC_SUBST([DIME_EXTRA_LDFLAGS], [$LDFLAGS])
AC_SUBST([DIME_EXTRA_LIBS], [$LIBS])

dnl *******************************************************************
dnl * Miscellaneous options and initializations.
dnl *******************************************************************

AM_INIT_AUTOMAKE(libdime, $VERSION)

dnl  Default to not building a static library.
dnl  Can be overridden by the user with --enable-static.
AM_DISABLE_STATIC

dnl  Turn off default maintainer make-rules -- use ./autogen.sh instead.
AM_MAINTAINER_MODE

if $BUILD_WITH_MSVC; then
  RANLIB=:
  export RANLIB
fi

dnl  Initialize libtool
AC_PROG_LIBTOOL

CPPFLAGS="$CPPFLAGS -DDIME_INTERNAL"

sim_ac_build_library=true

dime_build_dir=`pwd`
dime_src_dir=`cd "$srcdir"; pwd`


dnl *******************************************************************
dnl * Enable/disable compilation with debug information.
dnl *******************************************************************

AC_ARG_ENABLE(debug,
  [  --enable-debug          turn on Dime debugging [default=yes]],
  [case "${enableval}" in
    yes) enable_debug=yes ;;
    no)  enable_debug=no ;;
    *) AC_MSG_ERROR(bad value "${enableval}" for --enable-debug) ;;
  esac],
  enable_debug=yes)

if test "x$enable_debug" = "xno"; then
  CXXFLAGS="$CXXFLAGS -DNDEBUG"
fi


dnl *******************************************************************
dnl * Enable/disable compilation with symbol information.
dnl *******************************************************************

if test "x$ac_cv_prog_cxx_g" = "xyes"; then
  AC_ARG_ENABLE(symbols,
    [  --enable-symbols        turn on symbol debug information [default=yes]],
    [case "${enableval}" in
      yes) enable_symbols=yes ;;
      no)  enable_symbols=no ;;
      *) AC_MSG_ERROR(bad value "${enableval}" for --enable-symbols) ;;
    esac],
    enable_symbols=yes)

  if test "x$enable_symbols" = "xno"; then
    CXXFLAGS="`echo $CXXFLAGS | sed 's/-g//'`"
  fi
fi


dnl *******************************************************************
dnl * Enable/disable compilation with profiling information
dnl * for parsing by gprof (this is a g++ only option).
dnl *******************************************************************

if test "x$ac_cv_prog_cxx_g" = "xyes"; then
  AC_ARG_ENABLE(profile,
    [  --enable-profile        turn on inclusion of profiling code [default=no]],
    [case "${enableval}" in
      yes) enable_profile=yes ;;
      no)  enable_profile=no ;;
      *) AC_MSG_ERROR(bad value "${enableval}" for --enable-profile) ;;
    esac],
    enable_profile=no)

  if test "x$enable_profile" = "xyes"; then
    CXXFLAGS="$CXXFLAGS -pg"
    LDFLAGS="$LDFLAGS -pg"
  fi
fi

dnl *******************************************************************
dnl * Enable/disable compilation with warnings turned on (g++ only).
dnl *******************************************************************

if test "x$ac_cv_prog_gxx" = "xyes"; then
  AC_ARG_ENABLE(warnings,
    [  --enable-warnings       turn on warnings when compiling [default=no]],
    [case "${enableval}" in
      yes) enable_warnings=yes ;;
      no)  enable_warnings=no ;;
      *) AC_MSG_ERROR(bad value "${enableval}" for --enable-warnings) ;;
    esac],
    enable_warnings=no)

  if test "x$enable_warnings" = "xyes"; then
    CXXFLAGS="$CXXFLAGS -W -Wall"
  fi
fi

# **************************************************************************
# Headers we might want to use

AC_CHECK_HEADERS(
  [ieeefp.h], 
  [], [], 
  [ ])

# **************************************************************************

# We want to use BSD 4.3's isinf(), isnan(), finite() if they are
# available.

AC_MSG_CHECKING([for BSD 4.3 isinf() function])
AC_TRY_LINK(
 [#include <math.h>],
 [int result = isinf(42.0);],
 [AC_DEFINE(HAVE_ISINF, 1, [whether or not isinf() is available])
  AC_MSG_RESULT([available])],
 [AC_MSG_RESULT([not available])])

AC_MSG_CHECKING([for BSD 4.3 isnan() function])
AC_TRY_LINK(
 [#include <math.h>],
 [int result = isnan(42.0);],
 [AC_DEFINE(HAVE_ISNAN, 1, [whether or not isnan() is available])
  AC_MSG_RESULT([available])],
 [AC_MSG_RESULT([not available])

  AC_MSG_CHECKING([for _isnan() function])
  AC_TRY_LINK(
   [#include <math.h>],
   [int result = _isnan(42.0);],
   [AC_DEFINE(HAVE__ISNAN, 1, [whether or not _isnan() is available])
    AC_MSG_RESULT([available])],
   [AC_MSG_RESULT([not available])])
])

AC_MSG_CHECKING([for BSD 4.3 finite() function])
AC_TRY_LINK(
 [#include <math.h>],
 [int result = finite(42.0);],
 [AC_DEFINE(HAVE_FINITE, 1, [whether or not finite() is available])
  AC_MSG_RESULT([available])],
 [AC_MSG_RESULT([not available])

  AC_MSG_CHECKING([for _finite() function])
  AC_TRY_LINK(
   [#include <math.h>],
   [int result = _finite(42.0);],
   [AC_DEFINE(HAVE__FINITE, 1, [whether or not _finite() is available])
    AC_MSG_RESULT([available])],
   [AC_MSG_RESULT([not available])])
])

AC_MSG_CHECKING([for fpclass() function])
AC_TRY_LINK(
[#ifdef HAVE_IEEEFP_H
#include <ieeefp.h>
#endif],
 [int result = fpclass(42.0);],
 [AC_DEFINE(HAVE_FPCLASS, 1, [whether or not fpclass() is available])
  AC_MSG_RESULT([available])],
 [AC_MSG_RESULT([not available])])

AC_MSG_CHECKING([for _fpclass() function])
AC_TRY_LINK(
 [#include <math.h>],
 [int result = _fpclass(42.0);],
 [AC_DEFINE(HAVE__FPCLASS, 1, [whether or not _fpclass() is available])
  AC_MSG_RESULT([available])],
 [AC_MSG_RESULT([not available])])


# **************************************************************************

SIM_AC_UNIQIFY_LIST(DIME_EXTRA_CPPFLAGS, -I$includedir $DIME_EXTRA_CPPFLAGS)
SIM_AC_UNIQIFY_LIST(DIME_EXTRA_LDFLAGS, -L$libdir $DIME_EXTRA_LDFLAGS)
SIM_AC_UNIQIFY_LIST(DIME_EXTRA_LIBS, $DIME_EXTRA_LIBS)
SIM_AC_UNIQIFY_LIST(CPPFLAGS, $CPPFLAGS)
SIM_AC_UNIQIFY_LIST(LDFLAGS, $LDFLAGS)
SIM_AC_UNIQIFY_LIST(LIBS, $LIBS)

# **************************************************************************

# no DLL setup yet
case $enable_static in
yes | true)  DIME_STATIC=true ;;
*)           DIME_STATIC=false ;;
esac

case $enable_shared in
yes | true)  DIME_STATIC=false ;;
*)           DIME_STATIC=true ;;
esac

AC_SUBST([LIBFLAGS], [])

# **************************************************************************

AM_CONDITIONAL([BUILD_WITH_MSVC], [$BUILD_WITH_MSVC])

if $sim_ac_build_library; then
  if $BUILD_WITH_MSVC; then
    rm -f vc60.pdb
    LIBS="$LIBS $sim_ac_msvcrt_LIBLIBS"
    LIBFLAGS="$LIBFLAGS" # $sim_ac_msvcrt_LIBLDFLAGS
    if $DIME_STATIC; then
      DIME_EXTRA_CPPFLAGS="$DIME_EXTRA_CPPFLAGS -DDIME_NOT_DLL"
    else
      CPPFLAGS="-DDIME_MAKE_DLL $CPPFLAGS"
      DIME_EXTRA_CPPFLAGS="$DIME_EXTRA_CPPFLAGS -DDIME_DLL"
    fi
  
    debugfile="`pwd`/src/dime${DIME_MAJOR_VERSION}${SUFFIX}.pdb"
    debugfile=`cygpath -w "$debugfile" | sed 's,\\\\,\\\\\\\\,g'`
  
    if $DIME_STATIC; then
      SIM_AC_CONFIGURATION_SETTING([Dime build type], [static .lib])
      LIBFLAGS="-LIB $LIBFLAGS"
      LIBFLAGS="$LIBFLAGS /OUT:dime$DIME_MAJOR_VERSION$SUFFIX.lib"
    else
      LIBFLAGS="-DLL /INCREMENTAL:NO $LIBFLAGS"
      case $enable_symbols in
      no | false)
        SIM_AC_CONFIGURATION_SETTING([Dime build type],
                                     [dynamic .dll, no symbols])
        LIBFLAGS="$LIBFLAGS /RELEASE"
        ;;
      *)
        SIM_AC_CONFIGURATION_SETTING([Dime build type],
                                     [dynamic .dll + .pdb])
        CFLAGS="/Fd$debugfile -g $CFLAGS"
        CXXFLAGS="/Fd$debugfile -g $CXXFLAGS"
        LIBFLAGS="$LIBFLAGS /DEBUG /PDB:dime$DIME_MAJOR_VERSION$SUFFIX.pdb"
        ;;
      esac
      LIBFLAGS="$LIBFLAGS /OUT:dime$DIME_MAJOR_VERSION$SUFFIX.dll"
    fi
    SIM_AC_CONFIGURATION_SETTING([C library linkage], [$sim_ac_msvcrt])

    # We use a version suffix on the .dll-file, so several incompatible
    # (major) versions can be installed on a system.
    #
    # BTW, when linking DLLs, the 3rd-party .lib files will be
    # linked into the .dll file. I believe it is still advisable to
    # list all libs used upon `dime-config --libs`, as we can then
    # also use them from "parent" code (remember that their interfaces
    # is not exposed from the DLL) without any fuss.
    if test -n "$DIME_EXTRA_LIBS"; then
      DIME_EXTRA_LIBS="-ldime${DIME_MAJOR_VERSION}${SUFFIX} $DIME_EXTRA_LIBS"
    else
      DIME_EXTRA_LIBS="-ldime${DIME_MAJOR_VERSION}${SUFFIX}"
    fi
  else
    if test -n "$DIME_EXTRA_LIBS"; then
      DIME_EXTRA_LIBS="-ldime${SUFFIX} $DIME_EXTRA_LIBS"
    else
      DIME_EXTRA_LIBS="-ldime${SUFFIX}"
    fi
  fi

  DIME_COMPILER="$CXX"

  if $BUILD_WITH_MSVC; then
    # we'll use the installed wrapper when we use the dime-config script later
    # so we can remove the source code hierarchy
    DIME_COMPILER="wrapmsvc"
  fi
  AC_SUBST(DIME_COMPILER)

  sim_ac_path_problem=true # until proven otherwise
  if test -d "${bindir}"; then
    # search by inode seems safer than strings because of noncanonical paths
    ( findinode=`ls -id "$prefix/bin" 2>/dev/null | sed 's/^\([ ]*\)\([0-9]*\).*/\2/g'`
      IFS=:; for dir in $PATH; do
        inode=`ls -id "$dir" 2>/dev/null | sed 's/^\([ ]*\)\([0-9]*\).*/\2/g'`;
        test x"$inode" = x"$findinode" && exit 0
      done; exit 1 ) && sim_ac_path_problem=false
  fi
  if $sim_ac_path_problem; then
    ( IFS=:; for dir in $PATH; do
        test x"$dir" = x"${bindir}" && exit 0
      done; exit 1 ) && sim_ac_path_problem=false
  fi
  if $sim_ac_path_problem; then
    SIM_AC_CONFIGURATION_WARNING([Your \$PATH variable does not appear to include \"$prefix/bin\"])
  fi

else
  SIM_AC_CONFIGURATION_SETTING([Dime build type], [build disabled])
fi

# **************************************************************************

SIM_AC_MSVC_DSP_SETUP([DIME], [Dime], [dime], [-I$dime_build_dir -I$dime_build_dir/include -I$dime_src_dir/include])

# **************************************************************************

AM_CONFIG_HEADER([include/config.h])

AC_CONFIG_FILES([
	Makefile
	bin/Makefile
	src/Makefile
	src/classes/Makefile
	src/entities/Makefile
	src/objects/Makefile
	src/records/Makefile
	src/sections/Makefile
	src/tables/Makefile
	src/util/Makefile
	src/convert/Makefile
	dxf2vrml/Makefile
	dxfsphere/Makefile
])

AC_OUTPUT

# SIM_AC_CONFIGURATION_SUMMARY

if $sim_ac_build_library; then
  echo ""
  echo "Now, run 'make install' to build and install Dime."
  echo ""
else
  echo ""
fi

