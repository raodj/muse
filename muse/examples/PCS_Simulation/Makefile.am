
include $(top_srcdir)/Makefile.global.am

AM_CXXFLAGS += -I../../include

bin_PROGRAMS = pcs

pcs_LDFLAGS = -L../../kernel
pcs_LDADD = $(STDCPP) -lmuse

pcs_SOURCES = \
	PCS_Simulation.cpp\
	PCS_Simulation.h\
	PCSAgent.h\
	PCSAgent.cpp\
	PCS_State.h\
	PCS_State.cpp\
	PCSEvent.cpp\
	PCSEvent.h
	

# end of Makefile.am