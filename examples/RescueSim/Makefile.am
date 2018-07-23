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

AM_CXXFLAGS += -I./tools -I../../include -I./agents/include -I./events/include -I./states/include

bin_PROGRAMS = RescueSim

RescueSim_LDFLAGS = -L../../kernel $(AM_LDFLAGS)
RescueSim_LDADD = $(STDCPP) -lmuse
RescueSim_DEPENDENCIES=../../kernel/libmuse.a

RescueSim_SOURCES = \
	agents/include/RescueArea.h\
	agents/include/Victim.h\
	agents/include/Volunteer.h\
	agents/src/RescueArea.cpp\
	agents/src/Victim.cpp\
	agents/src/Volunteer.cpp\
	events/include/RescueEvent.h\
	events/include/UpdateNearbyEvent.h\
	events/include/UpdatePositionEvent.h\
	events/include/VolunteerEvent.h\
	events/src/UpdateNearbyEvent.cpp\
	events/src/UpdatePositionEvent.cpp\
	events/src/VolunteerEvent.cpp\
	events/src/RescueEvent.cpp\
	states/include/RescueAreaState.h\
	states/include/VictimState.h\
	states/include/VolunteerState.h\
	states/src/RescueAreaState.cpp\
	states/src/VictimState.cpp\
	states/src/VolunteerState.cpp\
	tools/VolunteerDataTypes.h\
	RescueSim_main.cpp\
	RescueSim_main.h\
	RescueSimPbs.sh

# end of Makefile.am
