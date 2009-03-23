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

AC_PREREQ(2.59)
AC_INIT([muse],[0.1.0],[raodm@muohio.edu])
AM_INIT_AUTOMAKE([-Wall -Werror subdir-objects])
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])
AC_REQUIRE_CPP

# The macros above automatically fill in some CXXFLAGS so we'll undo it.
CXX=mpicxx
CFLAGS=""
CXXFLAGS=""
CPPFLAGS=""

# Define MPI library (http://www.open-mpi.org)
AC_DEFUN([AC_PROG_MPI],
[
    AC_ARG_WITH(mpi,
            AS_HELP_STRING(--with-mpi=PATH,path to mpicc),
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
         AC_PATH_PROG([MPI], [mpicxx], [no])
     fi

     if test $MPI = no; then
        AC_MSG_ERROR([MPI could no be found on your system.
		      You can download MPI for free from
		      http://www.open-mpi.org
                      Please check that MPI is installed on your system
		      and your path includes the top-level install directory
		      (example: /usr/include/openmpi), or specify
		      --with-mpi=PATH option when running configure.])
     fi
     AC_SUBST(MPI)
])

# Check if mpicxx requires -lstdc++ option (needed on redhawk)
AC_DEFUN([AC_LINK_STDCPP],
[
     AC_MSG_CHECKING([if $CXX needs stdc++])
     AC_LANG([C++])
     AC_LANG_CONFTEST()
     
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
    AC_ARG_WITH(icc,
            AS_HELP_STRING(--with-icc,Code will be compiled with icc),
    [ if test "$withval" != "yes"; then
	  icc="yes"
      else
	  icc="no"
      fi
     ])

     if test -z "$icc"; then
         AC_MSG_CHECKING([for icc])
         icc=`mpicxx --version | grep "^icc"`
	 if test -n "$icc"; then
	     icc="yes"
	 else
	     icc="no"
	 fi
         AC_MSG_RESULT([$icc])
     fi

     if test "yes" == "$icc"; then
          ICC_NO_WARN_SOME="-wd981 -wd383 -wd424"
     else
          ICC_NO_WARN_SOME=""
     fi
     
     AC_SUBST(ICC_NO_WARN_SOME)
])

# Checks for programs.
AC_PROG_ICC
AC_LINK_STDCPP
AC_PROG_CXX
AC_PROG_MPI
AC_MSG_CHECKING([properties of MPI compiler])
AC_MSG_RESULT([$CXX])
AC_PROG_RANLIB

# Put object files in corresponding directories even if make is not
# running directly in the source directory.
AM_PROG_CC_C_O

# Checks for libraries.
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([m],[asin])
AC_SEARCH_LIBS([gethostname], [c,nsl])
AC_SEARCH_LIBS([bind],[c,nsl,socket])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h limits.h malloc.h netdb.h netinet/in.h stddef.h stdlib.h string.h strings.h sys/socket.h sys/time.h unistd.h values.h GL/glut.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_C_VOLATILE
AC_C_RESTRICT
AC_STRUCT_TM

# check if recent version of hash map is available in c++ land
AC_LANG_PUSH([C])
AC_LANG([C++])
AC_CHECK_HEADERS([tr1/unordered_map])
AC_LANG_POP([C++])

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_TYPE_SIGNAL
AC_FUNC_STRTOD
AC_CHECK_FUNCS([bzero floor gethostbyname gethostname gethrtime gettimeofday memset mkdir modf pow rint socket sqrt strcasecmp strchr strdup strstr strtol])

AC_CONFIG_FILES([Makefile\
	kernel/Makefile\
	examples/Makefile\
	examples/PingPongSimulation/Makefile\
	examples/RollbackHeavySimulation/Makefile\
	examples/LonelySimulation/Makefile\
	examples/RoundRobinSimulation/Makefile\
	examples/PHOLDSimulation/Makefile\
	examples/BugLife/Makefile\
	examples/ClockExample/Makefile])

AC_OUTPUT

# End of script
