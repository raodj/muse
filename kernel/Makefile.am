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
# Authors: Dhananjai M. Rao       raodm@muohio.edu
#
#---------------------------------------------------------------------------

include $(top_srcdir)/Makefile.global.am

AM_CXXFLAGS += -Iinclude -Isrc
# AM_CXXFLAGS += -DDEBUG_OUTPUT

noinst_LIBRARIES = libmuse.a

# libmuse_a_LDFLAGS = 
libmuse_a_LIBADD = $(STDCPP)

# Setup conditional compilation of OpenCL components, if OpenCL is
# available for use.
if COND_USE_OPENCL
    OPENCL_SOURCES = \
	include/ocl/oclAgent.h \
	include/ocl/oclScheduler.h \
	include/ocl/oclSimulation.h \
	include/ocl/OclSimulation.h \
	include/ocl/oclState.h \
	src/ocl/oclAgent.cpp \
	src/ocl/oclScheduler.cpp \
	src/ocl/oclSimulation.cpp \
	src/ocl/oclState.cpp
endif

libmuse_a_SOURCES = \
	include/EventAdapter.h \
	include/EventQueue.h \
	include/AgentPQ.h \
	include/BinaryHeapWrapper.h \
	include/Communicator.h \
	include/Compatibility.h \
	include/GVTManager.h \
	include/GVTMessage.h \
	include/HashMap.h \
	include/LinuxHashMap.h \
	include/Scheduler.h \
	include/WindowsHashMap.h \
	include/MPIHelper.h \
	include/EventRecycler.h\
	include/StateRecycler.h\
	include/NumaMemoryManager.h \
	src/Utilities.cpp \
	src/Agent.cpp \
	src/Event.cpp \
	src/Simulation.cpp \
	src/Communicator.cpp \
	src/Scheduler.cpp \
	src/State.cpp \
	src/Compatibility.cpp \
	src/GVTMessage.cpp \
	src/GVTManager.cpp \
	src/oSimStream.cpp \
	src/SimStream.cpp \
	src/MTRandom.cpp \
	src/BinaryHeapWrapper.cpp \
	src/AgentPQ.cpp \
	src/ArgParser.cpp \
	src/HRMScheduler.cpp \
	src/MPIHelper.cpp \
	src/EventRecycler.cpp \
	src/StateRecycler.cpp \
	src/NumaMemoryManager.cpp \
	src/EventQueue.cpp \
	include/HRMScheduler.h \
	include/ResChannel.h \
	src/ResChannel.cpp \
	include/LadderQueue.h \
	src/LadderQueue.cpp \
	include/HeapEventQueue.h \
	src/HeapEventQueue.cpp \
	include/TwoTierHeapEventQueue.h \
	src/TwoTierHeapEventQueue.cpp \
	include/TwoTierHeapOfVectorsEventQueue.h \
	src/TwoTierHeapOfVectorsEventQueue.cpp \
	include/TwoTierHeapAdapter.h \
	src/TwoTierHeapAdapter.cpp \
	include/ThreeTierHeapEventQueue.h \
	src/ThreeTierHeapEventQueue.cpp \
	include/TwoTierLadderQueue.h \
	src/TwoTierLadderQueue.cpp \
	include/DefaultSimulation.h \
	src/DefaultSimulation.cpp \
	include/SpinLock.h \
	include/mpi-mt/MultiThreadedCommunicator.h \
	src/mpi-mt/MultiThreadedCommunicator.cpp \
	include/mpi-mt/MultiThreadedSimulation.h \
	src/mpi-mt/MultiThreadedSimulation.cpp \
	include/mpi-mt/MultiThreadedSimulationManager.h \
	src/mpi-mt/MultiThreadedSimulationManager.cpp \
	include/mpi-mt/MTQueue.h \
	include/mpi-mt/SingleBlockingMTQueue.h \
	src/mpi-mt/SingleBlockingMTQueue.cpp \
	include/mpi-mt/MultiBlockingMTQueue.h \
	src/mpi-mt/MultiBlockingMTQueue.cpp \
	include/mpi-mt/MultiNonBlockingMTQueue.h \
	src/mpi-mt/MultiNonBlockingMTQueue.cpp \
	include/mpi-mt/RedistributionMessage.h \
	src/mpi-mt/RedistributionMessage.cpp \
	$(OPENCL_SOURCES)

# end of Makefile.am
