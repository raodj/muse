
include $(top_srcdir)/Makefile.global.am

AM_CXXFLAGS += -I../../include

bin_PROGRAMS = pcs

pcs_LDFLAGS = -L../../kernel
pcs_LDADD = $(STDCPP) -lmuse
pcs_DEPENDENCIES=../../kernel/libmuse.a

pcs_SOURCES = \
	PCS_Simulation.cpp\
	PCS_Simulation.h\
	PCS_Agent.h\
	PCS_Agent.cpp\
	PCS_State.h\
	PCS_State.cpp\
	PCS_Event.cpp\
	PCS_Event.h


# end of Makefile.am
