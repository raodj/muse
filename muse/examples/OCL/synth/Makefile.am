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

# AM_CPPFLAGS += -DCOMPARTMENTS=$(COMPARTMENTS)
AM_CXXFLAGS += -I../../../include

bin_PROGRAMS = synth

synth_LDFLAGS = -L../../../kernel -L $(AM_LDFLAGS)
synth_LDADD = $(STDCPP) -lmuse
synth_DEPENDENCIES=../../../kernel/libmuse.a 

nodist_synth_SOURCES = OclEqnSolvers.c_inc SynthKernel.c_inc
synth_CLEANFILES = OclEqnSolvers.c_inc SynthKernel.c_inc
BUILT_SOURCES = OclEqnSolvers.c_inc SynthKernel.c_inc

synth_SOURCES = SynthSimulation.h\
	SynthSimulation.cpp\
	OclEqnSolvers.h\
	SynthAgentState.h\
	SynthAgentState.cpp\
	SynthAgent.h\
	SynthAgent.cpp\
	OclEqnSolvers.c

# Custom build rules to generate an C++11 raw string version of header
# files to be incldued in the source file for generating OpenCL kernel
OclEqnSolvers.c_inc: OclEqnSolvers.c
	echo 'R"(' > $@
	cat $? >> $@
	echo ')"' >> $@

# Custom build rules to generate an C++11 raw string version of header
# files to be incldued in the source file for generating OpenCL kernel
SynthKernel.c_inc: SynthKernel.c
	echo 'R"(' > $@
	cat $? >> $@
	echo ')"' >> $@

# end of Makefile.am
