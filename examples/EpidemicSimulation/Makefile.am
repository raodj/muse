
include $(top_srcdir)/Makefile.global.am

AM_CXXFLAGS += -I../../include

bin_PROGRAMS = epidemic

epidemic_LDFLAGS = -L../../kernel
epidemic_LDADD = $(STDCPP) -lmuse
epidemic_DEPENDENCIES=../../kernel/libmuse.a

epidemic_SOURCES = \
	EpidemicEvent.cpp\
	EpidemicEvent.h\
	Location.cpp\
	Location.h\
	LocationState.cpp\
	LocationState.h\
	WattsStrogatz.h\
	Person.h\
	DiseaseModel.h

# end of Makefile.am

