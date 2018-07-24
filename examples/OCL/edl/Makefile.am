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
# Authors: Dhananjai M. Rao            raodm@miamiOH.edu
#
#---------------------------------------------------------------------------

include $(top_srcdir)/Makefile.global.am

AM_CXXFLAGS += -I../../../include

bin_PROGRAMS = edl_parser

edl_parser_LDFLAGS =
edl_parser_LDADD = $(STDCPP)
edl_parser_DEPENDENCIES = edl_parser.h.gch

edl_parser_CLEANFILES = edl_parser.h.gch
BUILT_SOURCES = edl_parser.h.gch

edl_parser_SOURCES = edl_parser.h\
	edl_parser.cpp\
	EdlAst.cpp\
	AgentGenerator.h\
	AgentGenerator.cpp\
	Generator.h\
	Generator.cpp\
	StateGenerator.h\
	StateGenerator.cpp\
	SimulationGenerator.h\
	SimulationGenerator.cpp\
	MakefileGenerator.h\
	MakefileGenerator.cpp

# Custom build rule to generate a precompiled header to keep
# compilation using BOOST Spirit to be fast.
edl_parser.h.gch: edl_parser.h
	$(CXXCOMPILE) $? $<

# end of Makefile.am
