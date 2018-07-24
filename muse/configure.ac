#   -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

#---------------------------------------------------------------------------
#
# Copyright (c) Miami University, Oxford, OH.
# All rights reserved.
#
# Miami University (MU) makes no representations or warranties about
# the suitability of the software, either express or implied,
# including but not limited to the implied warranties of
# merchantability, fitness for a particular purpose, or
# non-infringement.  MU shall not be liable for any damages suffered
# by licensee as a result of using, result of using, modifying or
# distributing this software or its derivatives.
#
# By using or copying this Software, Licensee agrees to abide by the
# intellectual property laws, and all other applicable laws of the
# U.S., and the terms of this license.
#
# Authors: Dhananjai M. Rao       raodm@muohio.edu
#
#---------------------------------------------------------------------------

AC_PREREQ(2.69)
AC_INIT([muse],[0.1.0],[raodm@muohio.edu])
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_REQUIRE_CPP

# The macros above automatically fill in some CXXFLAGS so we'll undo it.
# CXX=mpicxx
CFLAGS=""
CXXFLAGS=""
CPPFLAGS=""

# Define MPI library (http://www.open-mpi.org)
AC_DEFUN([AC_PROG_MPI],
[
    AC_ARG_WITH(mpi, AC_HELP_STRING(--with-mpi=PATH,path to mpicc),
        [ if test "$withval" != "yes"; then
              AC_MSG_CHECKING([for MPI])
	      if test -x "$withval" ; then
	          MPI="$withval"
	          AC_MSG_RESULT([$MPI])
	      else
	          MPI="no"
	          AC_MSG_RESULT(no)
	      fi
          fi
     ])
     
     if test -z "$MPI"; then
         AC_PATH_PROG([MPI], [mpicxx], [notFound])
     fi
     
     # have_mpi is set here and used in AC_CHECK_MPICC
     have_mpi=0

     if test $MPI = notFound; then
        AC_MSG_ERROR([MPI could no be found on your system.
                      To compile without MPI use --without-mpi option.
		      You can download MPI for free from
		      http://www.open-mpi.org
                      Please check that MPI is installed on your system
		      and your path includes the top-level install directory
		      (example: /usr/include/openmpi), or specify
		      --with-mpi=PATH option when running configure.])
     elif test $MPI = no; then
          AC_MSG_WARN([You have chosen to compile MUSE without MPI.
                       This is OK and MUSE will run in single-process mode
                       (MPI is needed to run in parallel using multiple
                       processes to fully utilize computational power of
                       supercomputing clusters). You can download MPI for
                       free from http://www.open-mpi.org])
     else
        CXX=mpicxx
        have_mpi=1
        AC_SUBST(MPI)
        AC_DEFINE([HAVE_LIBMPI], [1], [Using MPI to compile MUSE])
     fi
])

# A custom helper to check to ensure C++ version of MPI is operational
# Call AC_PROG_MPI before calling this check
AC_DEFUN([AC_CHECK_MPICC],
[
    if test -n "$have_mpi" && test $have_mpi = 1; then
        AC_MSG_CHECKING([whether $CXX is operational])
	AC_LANG([C++])
     
	AC_LINK_IFELSE(
            [AC_LANG_PROGRAM([[#include "mpi.h"
                               #include <cstdlib>
                             ]],
                             [[int argc=0; char **argv = 0;
                               MPI_Init(&argc, &argv); MPI_Finalize();
                             ]])],
	    [AC_MSG_RESULT([yes])],
            [AC_MSG_RESULT([no])
  	        AC_MSG_ERROR([
	            There is a serious configuration issue on this
	            machine.  It claims MPI is installed and mpicc is
	            your path. However, mpicc is not operational and
	            can't even compile a simple MPI program. This is
	            not a problem with MUSE. Most likely no MPI
	            program will compile on this machine.  Ensure you
	            are loading the correct version of MPI (possibly
	            using the 'module' command (example: module load
	            openmpi-x86_64) in ~/.bashrc])
        ])
    fi
])

# A custom helper macro check if NUMA development libraries are
# available and operational.  MUSE's multi-threaded mode is NUMA-aware
# and relies on Linux's NUMA libraries for its's operation.
AC_DEFUN([AC_PROG_NUMA],
[
     # Let user provide configure flag to enable/disable NUMA
     AC_ARG_WITH([numa],
         AS_HELP_STRING([--with-numa],[Enable NUMA aware memory management]),
         [ numa="$withval" ])

     # If user has not explicitly enabled/disabled NUMA then use numa
     # if libraries are found to be operational.
     if test -z "$numa"; then
         AC_MSG_CHECKING([whether NUMA library is operational])
         AC_LANG([C++])
         old_libs="$LIBS"
         LIBS="$LIBS -lnuma"
         AC_RUN_IFELSE(
             [AC_LANG_PROGRAM([[#include <numa.h>
                                #include <numaif.h>
                                #include <iostream>
                              ]],
                              [[return (numa_available() == -1 ? 1 : 0)]])],
	     [ AC_MSG_RESULT([yes])
               numa="yes" ],
             [ AC_MSG_RESULT([no])
               numa="no"
               LIBS="$old_libs"
             ]
         )
     fi

     # Now enable/disable compiler macro in config.h
     if test "$numa" != "yes"; then
         AC_DEFINE_UNQUOTED([USE_NUMA], 0, [Define to 1 if NUMA is usable.])
     else
         AC_DEFINE_UNQUOTED([USE_NUMA], 1, [Define to 1 if NUMA is usable.])
     fi
])

# Check if mpicxx requires -lstdc++ option (needed on redhawk)
AC_DEFUN([AC_LINK_STDCPP],
[
     AC_MSG_CHECKING([if $CXX needs stdc++])
     AC_LANG([C++])
     
     AC_LINK_IFELSE(
         [AC_LANG_PROGRAM([[#include "mpi.h"]],[[std::cout << "Hello";]])],
         [AC_MSG_RESULT([no])
	 ],
	 [AC_MSG_RESULT([yes])
	     LDFLAGS="-lstdc++ $LDFLAGS"
	 ])

])

# Check if we are using icc to supress a couple of useless remarks
AC_DEFUN([AC_PROG_ICC],
[
    AC_ARG_WITH([icc],
            AS_HELP_STRING([--with-icc],[Code will be compiled with icc]),
    [ if test "$withval" != "yes"; then
	  icc="yes"
      else
	  icc="no"
      fi
     ])

     if test -z "$icc"; then
         AC_MSG_CHECKING([for icc])
         icc=`$CXX --version | grep -iw "icc"`
	 if test -n "$icc"; then
	     icc="yes"
	 else
	     icc="no"
	 fi
         AC_MSG_RESULT([$icc])
     fi

     if test "yes" == "$icc"; then
          USING_ICC="-DICC"
          # -wd981: Supresses values evalutaed in differen order
          # -wd383: Supresses value copied to temporary for use
          # -wd424: Supresses extra ";" at end warning
          # -wd654: Supresses partial overload warnings
          ICC_NO_WARN_SOME="-wd981 -wd383 -wd424 -wd654"
	  ICC_NO_WARN_FOR_CONFIGURE="-wd981 -wd383 -wd1419"
     else
          USING_ICC=""
          ICC_NO_WARN_SOME=""
	  ICC_NO_WARN_FOR_CONFIGURE=""
     fi

     AC_SUBST(USING_ICC)
     AC_SUBST(ICC_NO_WARN_SOME)
])

# A custom helper macro check if OpenCL development libraries are
# available and operational.  MUSE's heterogeneous compute
# capabilities relies on OpenCL libraries for its's operation.
AC_DEFUN([AC_LIB_OPENCL],
[
    AC_ARG_WITH(opencl, AC_HELP_STRING(--with-opencl=PATH,path to OpenCL),
        [ if test "$withval" != "no"; then
              if test "$withval" != "yes"; then
                  AC_MSG_NOTICE([Checking for OpenCL in $withval])
                  OPEN_CL_PATH="$withval"
                  have_opencl="maybe"
              else
                  AC_MSG_NOTICE([Checking for OpenCL])
                  # Assume OpenCL is in default path
                  OPEN_CL_PATH=""
                  have_opencl="maybe"
              fi
          else
              # ignore and don't use OpenCL
              have_opencl="ignore"
          fi
        ], [
            # User has not specified any options. Ignore OpenCL by default
            have_opencl="ignore"
        ]
     )

     # Save current flags to restore after test below.
     OLD_CFLAGS="$CFLAGS"
     OLD_CPPFLAGS="$CPPFLAGS"
     LDFLAGS="$OLD_LDFLAGS"
     AC_LANG_PUSH([C++])

     # Check if we are able to compile a OpenCL headers
     if test "$have_opencl" = "maybe"; then
         # Add OpenCL path for use.
         CFLAGS="$CFLAGS -I$OPEN_CL_PATH/include"
         CPPFLAGS="$CPPFLAGS -I$OPEN_CL_PATH/include"
         AC_MSG_CHECKING([if OpenCL headers are operational])

         AC_COMPILE_IFELSE(
             [AC_LANG_PROGRAM([[#ifdef __APPLE__
                                #include <OpenCL/cl.hpp>
                                #else
                                #include "CL/cl.hpp"
                                #endif
                              ]],
                              [[cl::Platform::getDefault()]])],
             [AC_MSG_RESULT([yes])],
             [AC_MSG_RESULT([no])
              have_opencl="no"
             ])
     fi

     # Now that the header seems fine next check path to find library
     # and link against it.
     OPEN_CL_LDPATH=""     
     if test "$have_opencl" = "maybe"; then
         AC_MSG_CHECKING([for OpenCL library path])
         for subdir in "lib64" "lib/x86_64"; do
             ldpath="$OPEN_CL_PATH/$subdir"
             found="no"
             LDFLAGS="$OLD_LDFLAGS -L$ldpath -lOpenCL"
             AC_LINK_IFELSE(
                 [AC_LANG_PROGRAM([[#ifdef __APPLE__
                                    #include <OpenCL/cl.hpp>
                                    #else
                                    #include "CL/cl.hpp"
                                    #endif
                                  ]],  [[cl::Platform::getDefault()]])],
                  [found="yes"],  [found="no"])
             # If the library worked, we have found a path
             if test "$found" = "yes"; then
                 have_opencl="yes"
                 OPEN_CL_LDPATH="$ldpath"
                 break
             fi
         done
         if test "$have_opencl" = "yes"; then
             AC_MSG_RESULT([$OPEN_CL_LDPATH])
         else
             AC_MSG_RESULT([none])
             have_opencl="no"
         fi
     fi

     # Restore the search flags.
     AC_LANG_POP([C++])
     CFLAGS="$OLD_CFLAGS"
     CPPFLAGS="$OLD_CPPFLAGS"
     LDFLAGS="$OLD_LDFLAGS" 

     if test "$have_opencl" = "no"; then
        AC_MSG_ERROR([OpenCL headers were not found on your system.
                    Specify --with-opencl=PATH option when running configure.
                    To compile without OpenCL use --without-opencl option.])
     elif test "$have_opencl" = "ignore"; then
          AC_MSG_WARN([You have chosen to compile MUSE without OpenCL.
                    This is OK and MUSE will not include heterogeneous
                    computing capabilities])
     else
        OPEN_CL_PATH="-I$OPEN_CL_PATH/include"
        OPEN_CL_LDPATH="-L$OPEN_CL_LDPATH -lOpenCL"
        AC_SUBST(OPEN_CL_PATH)
        AC_SUBST(OPEN_CL_LDPATH)
        AC_DEFINE([HAVE_OPEN_CL], [1], [Using OpenCL in MUSE])
     fi
     # Setup conditional variable used in kernel/Makefile.am to
     # conditionally compile openCL components.
     AM_CONDITIONAL([COND_USE_OPENCL], [test "x$have_opencl" == "xyes"])
])

# A convenience macro to check if the compiler accepts a specific
# compiler flag or argument (and does not generate an error)
AC_DEFUN([AC_CHECK_ARGS],
[
    AC_MSG_CHECKING([whether _AC_LANG compiler accepts $1])
    if test -z "[$4]"; then
        AC_LANG_CONFTEST([AC_LANG_PROGRAM([[#include <iostream>]],
				          [[std::cout << "ac_check_args"]])])
        src=`ls conftest.c*`
        output=`$CXX $1 $CXXFLAGS $LIBS $ICC_NO_WARN_FOR_CONFIGURE $src 2>&1`
	flag_check=`expr match "$output" ".*$1.*"`
        if test "$flag_check" == "0"; then
            AC_MSG_RESULT([yes])
            m4_default([$2], :)
            if test -z "[$5]"; then 
                CPPFLAGS+=" [$1]"
            fi
        else
	    AC_MSG_RESULT([no])
	    m4_default([$3], :)
        fi
    else
        AC_MSG_RESULT([skipped (because "$4" overrides)])
	m4_default([$3], :)
    fi
])

# Check if we are using correct version of gcc
AC_DEFUN([AC_CHECK_GCC_VERSION],
[
    AC_MSG_CHECKING([whether gcc is sufficiently new])
    # Get version from GCC version header in the form:
    # gcc (GCC) 4.x.x ...
    version=`g++ --version 2>&1 | head -1 | cut -d' ' -f3`
    majorVer=`echo $version | cut -d'.' -f1`
    minorVer=`echo $version | cut -d'.' -f2`
    patchLevel=`echo $version| cut -d'.' -f3`

    good="yes"
    # Check to ensure gcc is at least 4.0.1
    if test $majorVer -lt 4; then
       good="no"
    elif test $majorVer -eq 4; then
	 if test $minorVer -lt 1 && test $patchLevel -lt 1; then
 	     good="no"
	 fi
	     # Add checks for buggy gcc 4.0.1+ versions here
    fi

    if test $good = no; then
        AC_MSG_RESULT([no])
	AC_MSG_ERROR([
	  MUSE has been developed using the latest C++ standards.
	  It requires atleast GCC Version 4.0.1 to compile. The gcc
	  version on your machine is $version which is not
	  sufficiently recent to compile MUSE. You need upgrade
	  gcc. If you are on a cluster environment contact your
	  system adminstrator to determine how to load a more recent
	  gcc module by default.
	 ])
    else
        AC_MSG_RESULT([yes, $version])
    fi
])

# Check if we are using gcc to supress a couple of useless remarks
AC_DEFUN([AC_PROG_GCC],
[
    AC_ARG_WITH(gcc,
        AC_HELP_STRING([--with-gcc],[Code will be compiled with gcc]),
        [ if test "$withval" == "yes"; then
	  	 gcc="yes"
          else
		gcc="no"
          fi
        ])

     if test -z "$gcc"; then
         AC_MSG_CHECKING([for gcc])
         gcc=`$CXX --version 2>&1 | grep -i -w "GCC"`
	 if test -n "$gcc"; then
	     gcc="yes"
	 else
	     gcc="no"
	 fi
         AC_MSG_RESULT([$gcc])
     fi

     if test "yes" == "$gcc"; then
         USING_GCC="-DGCC"
	 AC_CHECK_GCC_VERSION
     else
     	 USING_GCC=""
     fi

     AC_SUBST(USING_GCC) 
])

# A custom helper to check if a given version of hash map is
# operational.  The parameters to this macro call are:
# $1 The header file name being used (just for display purposes)
# $2 The defined to be set for compiler flags used to compile code
# $3 Flag to indicate if custom string hasher should be used.
# $4 Flag to indicate if this check is required in a series of checks.
AC_DEFUN([AC_CHECK_HASHMAP_OPTION],
[
     AC_MSG_CHECKING([whether hash map from $1 is operational])
     if test -z "[$4]"; then
	 old_cpp_flags=$CPPFLAGS
	 CPPFLAGS+=" -I include -I kernel/include"
         # Try to compile a simple hash map code with necessary headers
         AC_LINK_IFELSE(
             [AC_LANG_PROGRAM([[#define $2]
	               [#define $3]
                       [#include "HashMap.h"]],
		       [[const StringIntMap map; map.find("1")]])],
	 [AC_MSG_RESULT([yes]) # sucessful compile
	      found_hashmap=yes
              AC_DEFINE([$2], [1], [Version of hash map to use])	      
	      if test "[$3]" == "NEED_STRING_HASHER"; then
	          # String hasher is needed for successful compilation
	          AC_DEFINE([NEED_STRING_HASHER], [1],
		            [needs custom std::string hasher])
	      fi
	 ],
         [ AC_MSG_RESULT([no])
         ]) # outer link_ifelse
	 CPPFLAGS=$old_cpp_flags
    else
        AC_MSG_RESULT([skipped])
    fi
])

AC_DEFUN([AC_CHECK_HASHMAP],
[
    found_hashmap=""
    AC_LANG_PUSH([C++])
    
    # Check if the most recent unordered_map is usable.
    AC_CHECK_HASHMAP_OPTION([unordered_map], [HAVE_UNORDERED_MAP], [NO_STRING_HASHER], [$found_hashmap])
    AC_CHECK_HASHMAP_OPTION([unordered_map (custom hasher)], [HAVE_UNORDERED_MAP], [NEED_STRING_HASHER], [$found_hashmap])
    # Check if the previous iteration of tr1/unordered_map is usable.
    AC_CHECK_HASHMAP_OPTION([tr1/unordered_map], [HAVE_TR1_UNORDERED_MAP], [NO_STRING_HASHER], [$found_hashmap])
    AC_CHECK_HASHMAP_OPTION([tr1/unordered_map (custom hasher)], [HAVE_TR1_UNORDERED_MAP], [NEED_STRING_HASHER], [$found_hashmap])

    # Check if the older ext/hash_map is usable.
    AC_CHECK_HASHMAP_OPTION([ext/hash_map], [HAVE_EXT_HASH_MAP], [NO_STRING_HASHER], [$found_hashmap])
    AC_CHECK_HASHMAP_OPTION([ext/hash_map (custom hasher)], [HAVE_EXT_HASH_MAP], [NEED_STRING_HASHER], [$found_hashmap])

    # Finally fall back to default catch-all case
    AC_CHECK_HASHMAP_OPTION([hash_map], [HAVE_HASH_MAP], [NO_STRING_HASHER], [$found_hashmap])
    AC_CHECK_HASHMAP_OPTION([hash_map (custom hasher)], [HAVE_HASH_MAP], [NEED_STRING_HASHER], [$found_hashmap])
    
    AC_LANG_POP([C++])

    # If a useable hash map was not detected then report an error.
    if test -z "$found_hashmap"; then
        AC_MSG_ERROR([
	    Despite best efforts this machine does not seem to have an
	    operational version of hash_map or unordered_map. This
	    indicates your C++ compiler is not sufficiently recent to
	    be upto date with standards.  Possibly you are using a
	    very old C++ compiler and you need to upgrade it. If you
	    are on a cluster environment contact your administrator to
	    see if you can load a more recent C++ compiler module by
	    default
	])
    fi
])

# Checks for programs.
AC_PROG_MPI
AC_CHECK_MPICC
AC_MSG_CHECKING([properties of C++ compiler])
AC_MSG_RESULT([$CXX])
AC_PROG_CXX
AC_LINK_STDCPP
AC_PROG_ICC
AC_PROG_GCC
AC_PROG_RANLIB
AC_PROG_NUMA
AC_LIB_OPENCL

# Checks for suitable archiver to be used
AM_PROG_AR

# Check and set some default compiler flags for the build
AC_CHECK_ARGS([-Minform=warn], [INFORM_WARN="-Minform=warn"], [INFORM_WARN=""])
AC_SUBST(INFORM_WARN)
AC_CHECK_ARGS([-Wall], [WALL="-Wall"], [WALL=""], [])
AC_SUBST(WALL)
AC_CHECK_ARGS([-Wextra], [WEXTRA="-Wextra"], [WEXTRA=""], [])
AC_SUBST(WEXTRA)
AC_CHECK_ARGS([-pipe], [PIPE="-pipe"], [PIPE=""], [])
AC_SUBST(PIPE)

# The -fast flag for ICC causes libraries to break due to inter-procedural
# call optimizations. So sadly it cannot be used for libraries. 
# AC_CHECK_ARGS([-fast], [FAST="-fast"], [FAST=""], [])
# AC_SUBST(FAST)

# The -O3 flag is now accepted by gcc, icc, & clang.
AC_CHECK_ARGS([-O3], [OPT_O3="-O3"], [OPT_O3=""], [$OPT_O3], ["no"])

# Override -O3 based on developer assertions flags as needed.
DEV_ASSERT=""
AC_ARG_ENABLE([assertions],
    AS_HELP_STRING([--enable-assertions], [Enable developer assertion checks for sanity checking]),
    [ if test "$enableval" == "yes"; then
          AC_MSG_NOTICE([Enabling developer assertion compiler flag])    
          DEV_ASSERT="-DDEVELOPER_ASSERTIONS"
      fi
    ])
# override full debugging & optimization flags as needed.
AC_ARG_ENABLE([debug],
    AS_HELP_STRING([--enable-debug], [Setup all compile in debugging mode]),
    [ if test "$enableval" == "yes"; then
          AC_MSG_NOTICE([Enabling debug flags & developer assertions])
          OPT_O3="-O0"
          DEV_ASSERT="-DDEVELOPER_ASSERTIONS"
      fi
    ])

# override full debugging & optimization flags as needed.
AC_ARG_ENABLE([full-debug],
    AS_HELP_STRING([--enable-full-debug], [Setup all compile with debugging outputs]),
    [ if test "$enableval" == "yes"; then
          AC_MSG_NOTICE([Enabling full debug flags & developer assertions])    
          OPT_O3="-O0"
          DEV_ASSERT="-DDEVELOPER_ASSERTIONS -DDEBUG_OUTPUT"
      fi
    ])

# Substitute values of variables in makefile(s)
AC_SUBST(OPT_O3)
AC_SUBST(DEV_ASSERT)

# Check and start using new C++11 standards
# AC_CHECK_ARGS([-std=c++14], [CPP14="-std=c++14"], [CPP14=""], [])
# AC_SUBST(CPP14)
AC_CHECK_ARGS([-std=c++11], [CPP11="-std=c++11"], [CPP11=""], [$CPP14], ["no"])
AC_SUBST(CPP11)
AC_CHECK_ARGS([-std=c++0x], [CPP0X="-std=c++0x"], [CPP0X=""], [$CPP11], ["no"])
AC_SUBST(CPP0X)

# Put object files in corresponding directories even if make is not
# running directly in the source directory.
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([m],[asin],,,[-lstdc++])
# AC_CHECK_FUNCS([MPI_Init])
AC_SEARCH_LIBS([initscr],[curses],,,[$LINK_STD_CPP])
AC_CHECK_LIB([readline],[readline],,,[$LINK_STD_CPP])
AC_CHECK_LIB([pthread], [pthread_create],,,[$LINK_STD_CPP])
AC_SEARCH_LIBS([gethostname], [c,nsl],,,[$LINK_STD_CPP])
AC_SEARCH_LIBS([bind],[c,nsl,socket],,,[$LINK_STD_CPP])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([alloca.h cstdlib string pthread.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_VOLATILE
AC_C_RESTRICT
AC_STRUCT_TM


# Checks for library functions.
# AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STRTOD
AC_CHECK_FUNCS([floor memset mkdir pow pthread_getattr_np])

# Finally detect the most recent and working version of hash map is
# available in c++ land
AC_CHECK_HASHMAP

AC_CONFIG_FILES([Makefile\
	kernel/Makefile\
	examples/Makefile\
	examples/PingPongSimulation/Makefile\
	examples/RollbackHeavySimulation/Makefile\
	examples/LonelySimulation/Makefile\
	examples/RoundRobinSimulation/Makefile\
	examples/PHOLDSimulation/Makefile\
	examples/ClockExample/Makefile\
	examples/RescueSim/Makefile\
	examples/BugLife/Makefile\
	examples/PCS_Simulation/Makefile\
        examples/EpidemicSimulation/Makefile\
        examples/OCL/Makefile\
        examples/OCL/edl/Makefile\
        examples/OCL/synth/Makefile])

AC_OUTPUT

# End of script
