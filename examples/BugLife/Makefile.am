# This file is processed by automake to generate Makefile.in

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
# Authors: Meseret R. Gebre       meseret.gebre@muohio.edu
#
#---------------------------------------------------------------------------

include $(top_srcdir)/Makefile.global.am

AM_CXXFLAGS += -I../../include -Isrc

bin_PROGRAMS = bugsim

bugsim_LDFLAGS = -L../../kernel $(AM_LDFLAGS)
bugsim_LDADD = $(STDCPP) -lmuse
bugsim_DEPENDENCIES=../../kernel/libmuse.a

bugsim_SOURCES = \
	src/Bug.cpp\
	src/Bug.h\
	src/Space.cpp\
	src/Space.h\
	src/BugEvent.cpp\
	src/BugEvent.h\
	src/Born.cpp\
	src/Born.h\
	src/Dead.cpp\
	src/Dead.h\
	src/Eat.cpp\
	src/Eat.h\
	src/Grow.cpp\
	src/Grow.h\
	src/MoveIn.cpp\
	src/MoveIn.h\
	src/MoveOut.cpp\
	src/MoveOut.h\
	src/Scout.cpp\
	src/Scout.h\
	src/BugState.cpp\
	src/BugState.h\
	src/SpaceState.cpp\
	src/SpaceState.h\
	src/BugDataTypes.h\
	BugLifeSimulation.cpp\
	BugLifeSimulation.h\
	BugSimPbs.sh

# end of Makefile.am
