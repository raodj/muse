R"(#-*-makefile-*-

CC ?= gcc
CXX ?= g++
# Set flags to be the same as the one used by MUSE
MUSE_FLAGS =

# Typically rest of the Makefile does not need to change.

# Check to ensure the path to muse and opencl are setup
ifndef MUSE_PATH
$(error MUSE_PATH is not set. Set it to point to ~/research/muse)
endif

ifndef OPENCL_PATH
$(error OPENCL_PATH is not set. Set it to point to /usr/local/cuda/8.0.44)
endif

# Setup more compiler and linker flags used in this makefile
CFLAGS += -g -Wall -I$(MUSE_PATH)/include -DCOMPARTMENTS=6
CPPFLAGS += $(CFLAGS) -std=c99
CXXFLAGS += $(CFLAGS) -std=c++11 $(MUSE_FLAGS)
LIBS += -L $(OPENCL_PATH)/lib64 -lOpenCL -L $(MUSE_PATH)/kernel -lmuse -lpthread -lnuma


OBJ_DIR ?= obj

EDL_CPP_SRCS  = $(wildcard *.cpp)
EDL_C_SRCS = $(wildcard *.c)

EDL_CPP_OBJS = $(patsubst %.cpp, $(OBJ_DIR)/%.o, $(EDL_CPP_SRCS))
EDL_C_OBJS = $(patsubst %.c, $(OBJ_DIR)/%.o, $(EDL_C_SRCS))

# The following rules are not dependent on source files.
.PHONY: all depend cleandep clean

all: %MODEL%

$(OBJ_DIR):
	mkdir -p $@

%MODEL%: $(OBJ_DIR) $(EDL_CPP_OBJS) $(EDL_C_OBJS)
	$(CXX) $(CPPFLAGS) $(INCLUDES) $(EDL_CPP_OBJS) $(EDL_C_OBJS) $(LIBS) -o $@

# Rule to make all the .o files
$(OBJ_DIR)/%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(OBJ_DIR)/%.o : %.c
	$(CC) -c $(CPPFLAGS) $< -o $@

depend:
	rm -f .depend
	g++ -MM -MG $(MDFLAGS) $(CPPFLAGS) $(INCLUDES) $(EDL_SRCS) > .depend

cleandep:
	rm -f .depend

clean:
	rm -f *~ \#*\#\
	rm -rf $(OBJ_DIR)

-include .depend
)"
