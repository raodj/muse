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
# Authors: Meseret R. Gebre       meseret.gebre@gmail.com
#
#---------------------------------------------------------------------------

include $(top_srcdir)/Makefile.global.am

AM_CXXFLAGS += -I../../include

bin_PROGRAMS = phold

phold_LDFLAGS = -L../../kernel $(AM_LDFLAGS)
phold_LDADD = $(STDCPP) -lmuse
phold_DEPENDENCIES=../../kernel/libmuse.a

phold_SOURCES = \
	PHOLDSimulation.cpp\
	PHOLDSimulation.h\
	PHOLDAgent.h\
	PHOLDAgent.cpp\
	PholdState.h\
	PholdState.cpp\
	PHOLDEvent.h\
	P-HOLD-Simulation.doc\
	pholdTestPbs.sh\
	pholdTest.sh
# end of Makefile.am
