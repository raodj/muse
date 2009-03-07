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

AM_CXXFLAGS += -I../../include

bin_PROGRAMS = bugsim

bugsim_LDFLAGS = -L../../kernel
bugsim_LDADD = $(STDCPP) -lmuse

bugsim_SOURCES = \
	agents/src/Bug.cpp\
	agents/header/Bug.h\
	agents/src/Space.cpp\
	agents/header/Space.h\
	events/src/BugEvent.cpp\
	events/header/Bugevent.h\
	events/src/Eat.cpp\
	events/header/Eat.h\
	events/src/Grow.cpp\
	events/header/Grow.h\
	events/src/MoveIn.cpp\
	events/header/MoveIn.h\
	events/src/MoveOut.cpp\
	events/header/MoveOut.h\
	events/src/Scout.cpp\
	events/header/Scout.h\
	states/src/BugState.cpp\
	states/header/BugState.h\
	states/src/SpaceState.cpp\
	states/header/SpaceState.h\
	tools/arg_parser.cpp\
	tools/arg_parser.h\
	BugLifeSimulation.cpp	

# end of Makefile.am
