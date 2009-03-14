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

AM_CXXFLAGS += -wd 810 -I../../include -I./agents/include -I./events/include -I./states/include -I./tools

bin_PROGRAMS = bugsim

bugsim_LDFLAGS = -L../../kernel
bugsim_LDADD = $(STDCPP) -lmuse

bugsim_SOURCES = \
	agents/src/Bug.cpp\
	agents/src/Space.cpp\
	events/src/BugEvent.cpp\
	events/src/Eat.cpp\
	events/src/Grow.cpp\
	events/src/MoveIn.cpp\
	events/src/MoveOut.cpp\
	events/src/Scout.cpp\
	states/src/BugState.cpp\
	states/src/SpaceState.cpp\
	tools/arg_parser.cpp\
	BugLifeSimulation.cpp	

# end of Makefile.am
