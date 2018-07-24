#-*-makefile-*-

CXX ?= g++
OBJ_DIR ?= obj
CPPFLAGS += -g -Wall -std=c++11

EDL_SRCS = edl_parser.cpp\
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

EDL_OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(EDL_SRCS))

# The following rules are not dependent on source files.
.PHONY: all depend cleandep clean

all: edl_parser

# Make precompiled header to make repeated compiles go faster
edl_parser.h.gch: edl_parser.h
	@echo "Building precompiled header..."
	g++ $(CPPFLAGS) $(INCLUDES) $<

$(OBJ_DIR):
	mkdir -p $@

edl_parser: $(OBJ_DIR) edl_parser.h.gch $(EDL_OBJS)
	g++ $(CPPFLAGS) $(INCLUDES) $(LIBS) $(EDL_OBJS) -o $@

# Rule to make all the .o files
$(OBJ_DIR)/%.o : %.cpp
	$(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

depend:
	rm -f .depend
	g++ -MM -MG $(MDFLAGS) $(CPPFLAGS) $(INCLUDES) $(EDL_SRCS) > .depend

cleandep:
	rm -f .depend

clean:
	rm -f *~ \#*\#\
	rm -f edl_parser.h.gch
	rm -rf $(OBJ_DIR)

-include .depend
