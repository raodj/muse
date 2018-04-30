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

AM_CXXFLAGS += -I../../include  -I $$AMDAPPSDKROOT/include/ -I ../../kernel/include/ -I ../../kernel/include/ocl

bin_PROGRAMS = ocl

ocl_LDFLAGS = -L../../kernel -L $$AMDAPPSDKROOT/lib/x86_64 
ocl_LDADD = $(STDCPP) -lmuse -lOpenCL 
ocl_DEPENDENCIES=../../kernel/libmuse.a 

ocl_SOURCES = OclExampleSimulation.h\
	OclExampleSimulation.cpp\
	synth/SynthAgent.h\
	synth/SynthAgent.cpp\
	ebola/EbolaAgent.h\
	ebola/EbolaAgent.cpp

# end of Makefile.am
