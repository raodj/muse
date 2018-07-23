#ifndef MUSE_OCL_SIMULATION_CPP
#define MUSE_OCL_SIMULATION_CPP
//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors:  Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include <vector>
#include <string>
#include <algorithm>
#include <chrono>

#include "ocl/OclSimulation.h"
#include "ocl/OclBufferManager.h"
#include "Communicator.h"
#include "MPIHelper.h"
#include "ArgParser.h"
#include "GVTManager.h"
#include "HCAgent.h"

BEGIN_NAMESPACE(muse);

OclSimulation::OclSimulation(const bool useSharedEvents)
    : Simulation(useSharedEvents) {
    oclRunTime       = 0;
    lastLGVT         = 0;  // Previous LGVT value when OpenCL was run.
    maxWorkGroupSize = -1;
    isKernelRunning  = false;
    platformID       = 0;  // First platform found
    deviceID         = 0;  // First device on platfrom
    listOclDevices   = false;
}

void
OclSimulation::initAgents() {
    // First let the base class do standard initialization of all
    // agents.
    Simulation::initAgents();
    // Now agents that require heterogeneous computing (HC) operation
    // would have set their Kernel ID's in the muse::Agent::hcKernel
    // value.
    for (muse::Agent* agent : allAgents) {
        if (agent->hcKernel != -1) {
            // Have an agent who needs HC operations. So create a
            // kernel source code for it.  Currently, we handle only 1
            // type of kernel.  So the hcKernel should always be zero.
            ASSERT(agent->hcKernel == 0);
            // Build the OpenCL kernel/program for kernel if it is not
            // already available.  The following logic is not
            // right. Instead, we also need to check if prior kernels
            // are valid incase the we get kernel IDs in differen
            // orders
            if ((int) kernelList.size() <= agent->hcKernel) {
                buildOclKernel(dynamic_cast<HCAgent*>(agent),
                               agent->hcKernel);
            }
            agent->hcKernel = -1;  // reset
        }
    }
    // Initialize the rndGenBuf to all zeros on the device.
    OclBufferManager rndGenBufMgr(rndGenBuf, queue);
    struct MTrand_Info* rndInfo = rndGenBufMgr.map<struct MTrand_Info>(true);
        bzero(rndInfo, rndGenBufMgr.size());
    // Setup random seeds for each generator
    const int now = time(NULL);
    for (int i = 0; (i < maxWorkGroupSize); i++) {
        rndInfo[i].seed = now + i;
    }
}

bool
OclSimulation::processNextEvent() {
    // Update lgvt to the time of the next event to be processed.
    LGVT = scheduler->getNextEventTime();
    // Check and handle any open cl execution to be finished.
    if (lastLGVT > LGVT) {
        // Rollback has occurred. We need to cancel out pending OpenCL
        // execution.
        waitForOclKernel();
        // Reset the lastLGVT to handle rollback case
        lastLGVT = LGVT;
    }
    // Process any pending agents at given LVT prior to advancing to
    // next LVT.
    if ((LGVT > lastLGVT) || ((int) oclAgents.size() >= maxWorkGroupSize)) {
        // Either LVT has advanced or we have a lot of agents already
        // that can be scheduled. So do OpenCL processing now.
        runOclKernels((LGVT > lastLGVT));
        // Update time.
        lastLGVT = LGVT;
    }
    
    // Do sanity checks.
    if (LGVT < getGVT()) {
        std::cout << "Offending event: "
                  << *scheduler->agentPQ->front() << std::endl;
        std::cout << "LGVT = " << LGVT << " is below GVT: " << getGVT()
                  << " which is serious error. Scheduled agents: \n";
        scheduler->agentPQ->prettyPrint(std::cout);
        std::cout << "Rank " << myID << " Aborting.\n";
        std::cout << std::flush;
        DEBUG(logFile->close());
        abort();
    }
    // Let the scheduler do its task to have the agent process events
    // if possible.  If no events are processed the following method
    // call returns an InvalidAgent.  Otherwise the method below
    // returns with value, add agent to list of agents to be bulk
    // processed by gpu
    const AgentID ret = scheduler->processNextAgentEvents();
    // Check and add this agent to the list of agents to use OpenCL
    // later on.
    if ((ret != InvalidAgentID) && (allAgents[ret]->hcKernel != -1)) {
        // add agent to list of agents that need OpenCL kernel to be run
        Agent* agent = kernel->allAgents[ret];
        ASSERT(dynamic_cast<HCAgent*>(agent) != NULL);
        oclAgents.push_back(static_cast<HCAgent*>(agent));
    }
    return (ret != InvalidAgentID);
}

cl::Platform
OclSimulation::getPlatform(int platformID, bool display) const {
    // Find out the list of platforms available.
    std::vector<cl::Platform> platformList;
    cl::Platform::get(&platformList);
    // Print information about available devices on the platform to
    // help user identify suitable device to use.
    if (display) {
        for (size_t i = 0; i < platformList.size(); i++) {
            std::cout << "Platform " << i << ": "
                      << platformList[i].getInfo<CL_PLATFORM_NAME>()
                      << std::endl;
        }
    }
    // Check if we have the specified platform
    if ((int) platformList.size() <= platformID) {
        std::cerr << "OpenCL platform with ID: " << platformID << "not found.\n"
                  << "Number of platforms = " << platformList.size()
                  << std::endl;
        abort();
    }
    // Return information about the requested platform 
    return platformList.at(platformID);
}

cl::Device
OclSimulation::getDevice(cl::Platform platform, int deviceID,
                         bool display) const {
    std::vector<cl::Device> deviceList;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &deviceList);
    // Print information about available devices on the platform to
    // help user identify suitable device to use.
    if (display) {
        for (size_t j = 0; j < deviceList.size(); j++) {
            std::cout << "Device " << j << ": "
                      << deviceList[j].getInfo<CL_DEVICE_NAME>()
                      << std::endl;
        }
    }
    // Check if we have the specified device
    if ((int) deviceList.size() < deviceID) {
        std::cerr << "OpenCL device with ID: " << deviceID << "not found.\n"
                  << "Number of device = " << deviceList.size()
                  << std::endl;
        abort();
    }
    if (display) {
        std::cout << "Max workgroup size: "
                  << deviceList.at(deviceID).
            getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>() << std::endl;
    }
    return deviceList.at(deviceID);
}

void
OclSimulation::initOpenCL(int platformID, int deviceID, bool display) {
    // Initialize opencl platform and devices
    cl::Platform plat  = getPlatform(platformID, display);
    device             = getDevice(plat, deviceID, display);
    // Update max workgroup size
    if (maxWorkGroupSize == -1) {
        maxWorkGroupSize = device.getInfo<CL_DEVICE_MAX_WORK_GROUP_SIZE>();
    }
    // Initialize the context instance variable
    context            = cl::Context(device);
    // Setup the command queue
    queue              = cl::CommandQueue(context, device,
                                          CL_QUEUE_PROFILING_ENABLE);
    
    // Create the buffers used to send/receive agent IDs
    agentIDsBuf = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                             sizeof(muse::AgentID) * maxWorkGroupSize);
    paramBuf    = cl::Buffer(context, CL_MEM_READ_ONLY | CL_MEM_ALLOC_HOST_PTR,
                             10240);    
    stateBuf    = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                             10240);
    rndGenBuf   = cl::Buffer(context, CL_MEM_READ_WRITE | CL_MEM_ALLOC_HOST_PTR,
                             sizeof(MTrand_Info) * maxWorkGroupSize);

}

void
OclSimulation::initialize(int& argc, char* argv[], bool initMPI)
    throw(std::exception) {
    // Create a default communicator.
    commManager = new Communicator();
    myID = commManager->initialize(argc, argv, initMPI);
    unsigned int numThreads;  // dummy. not really used.
    commManager->getProcessInfo(myID, numberOfProcesses, numThreads);
    // Consume any specific command-line arguments used to setup and
    // configure other components like the scheduler and GVT manager.
    parseCommandLineArgs(argc, argv);    
    // First let the base class do the necessary initialization.
    muse::Simulation::initialize(argc, argv, initMPI);
    // Initialize OpenCL with specified platform and device
    initOpenCL(platformID, deviceID, listOclDevices);
}

void
OclSimulation::parseCommandLineArgs(int &argc, char* argv[]) {
    // First let the base class do the core processing.
    muse::Simulation::parseCommandLineArgs(argc, argv);
    // Now parse OCL arguments.
    ArgParser::ArgRecord arg_list[] = {
        {"--workgroup-size", "Number of agents run on GPU at a time",
         &maxWorkGroupSize, ArgParser::INTEGER},
        {"--platform", "The zero-based index of the platform to use",
         &platformID, ArgParser::INTEGER},
        {"--device", "The zero-based index of the device on platform to use",
         &deviceID, ArgParser::INTEGER},
        {"--ocl-list", "List platforms & devices found via OpenCL",
         &listOclDevices, ArgParser::BOOLEAN},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
}

void
OclSimulation::preStartInit() {
    // First let the base class do the necessary setup.
    muse::Simulation::preStartInit();
    // Next, we setup/finalize the AgentMap for all kernels
    commManager->registerAgents(allAgents);
}

cl::Kernel
OclSimulation::compileToKernel(const std::string& source,
                               const std::string& name) const
    throw (cl::Error) {
    // Compile the OpenCL source code
    cl::Program::Sources sources(1, std::make_pair(source.c_str(), 0));
    cl::Program program(context, sources);
    // Build Open CL program for a given device
    try {
        if (program.build({ device }, "-I.") != CL_SUCCESS) {
            throw cl::Error(CL_BUILD_PROGRAM_FAILURE);
        }
    } catch (cl::Error error) {
        std::cerr << "Build log:" << std::endl
                  << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device)
                  << "-------------[ kernel source below ]-------------\n"
                  << source << std::endl;
        throw error;
    }
    // Now that the program compiled, create and return a kernel
    return cl::Kernel(program, name.c_str());
}

void
OclSimulation::buildOclKernel(muse::HCAgent* agent, const int kernelID) {
    ASSERT( agent != NULL );
    if (kernelID >= (int) kernelList.size()) {
        // Grow kernel list to accommodate new kernel.
        kernelList.resize(kernelID + 1);
    }
    ASSERT( kernelID < (int) kernelList.size() );
    // Build the program in a string stream for convenience.
    std::ostringstream srcCode;    
    // Include pre-declaration helpers/definitions
    srcCode << MuseOclLibrary << std::endl;
    srcCode << agent->getHCPreSupportCode() << std::endl;
    // Next get the information for the state to be passed to this
    // kernel
    const HCState* state = dynamic_cast<const HCState*>(agent->getState());
    ASSERT( state != NULL );
    srcCode << state->getHCStateDefinition() << "\n\n";
    // Next get the information for the parameter structure to be
    // passed to this kernel.
    srcCode << agent->getHCParamDefinition();
    // Get all the supporting OCL code.
    srcCode << agent->getHCPostSupportCode();
    // Finally, get the user's top-level kernel code.
    srcCode << agent->getHCkernelDefinition();
    // Finally, add MUSE's OpenCL interface code
    srcCode << MuseOclKernel << "\n\n";    
    // Now compile the source code to an executable kernel
    kernelList[kernelID] = compileToKernel(srcCode.str());
}

void
OclSimulation::copyDataToDevice() {
    if (oclAgents.empty()) {
        return;  // No data to copy
    }
    // Copy agentID, state, and parameter information to device.
    OclBufferManager agentIDMgr(agentIDsBuf, queue),
        stateBufMgr(stateBuf, queue), paramBufMgr(paramBuf, queue);
    // Map the buffers for writing.
    const int agentCount    = oclAgents.size();
    muse::AgentID* agentIDs = agentIDMgr.map<muse::AgentID>(true,
                              agentCount * sizeof(muse::AgentID));
    // NOTE: In our implementation we assume that the state and
    // parameter sizes are the same for all of the agents.
    const int stateSize = static_cast<muse::HCState*>(oclAgents[0]->
                                            getState())->getHCStateSize();
    const int paramSize = oclAgents[0]->getHCParamSize();
    char* stateMem = stateBufMgr.map<char>(true, stateSize * agentCount);
    char* paramMem = paramBufMgr.map<char>(true, paramSize * agentCount);
    // Copy information for each agent.
    for (int i = 0, stateBufOffset = 0, paramBufOffset = 0;
         (i < agentCount); i++, stateBufOffset += stateSize,
             paramBufOffset += paramSize) {
        // Obtain direct reference to the agent and state to
        // streamline code below.
        muse::HCAgent* agent = oclAgents[i];
        ASSERT(dynamic_cast<muse::HCState*>(agent->getState()) != NULL);
        muse::HCState* state = static_cast<muse::HCState*>(agent->getState());
        // Copy the agent ID.
        agentIDs[i]   = agent->getAgentID();
        // Copy the parameter information to the buffer. All
        // parameters should be of the same size to keep OpenCL
        // interface fast and streamline its usage.
        ASSERT( agent->getHCParamSize() == paramSize );
        ASSERT((paramBufOffset + paramSize) <= paramBufMgr.size());
        // Get the state to copy data into the buffer
        agent->copyToDevice(paramMem + paramBufOffset, paramSize);
        // Copy the state to the buffer. All states currently should
        // be of the same size to keep OpenCL interface fast.
        ASSERT( state->getHCStateSize() == stateSize );
        ASSERT( (stateBufOffset + stateSize) <= stateBufMgr.size());
        // Get the state to copy data into the buffer
        state->copyToDevice(stateMem + stateBufOffset, stateSize);        
    }
    // Buffers are automatically unmapped in by the buffer managers.
}

void
OclSimulation::runOclKernels(bool blocking) {
    // First wait for all currently scheduled OCL kernels to finish
    // running.
    waitForOclKernel();
    ASSERT(!isKernelRunning);
    // If we don't have any pending agents, then do nothing.
    if (oclAgents.empty()) {
        return;  // No pending agents that require HC operations.
    }
    ASSERT((int) oclAgents.size() <= maxWorkGroupSize);
    // Copy the agentIDs, state, and parameter buffers to the device
    copyDataToDevice();
    // Update the list of agents to be run on the GPU.
    runningAgents = oclAgents;
    oclRunLGVT    = lastLGVT;
    // Setup all the arguments in the correct order!
    cl::Kernel& oclKernel = kernelList.at(0);
    // Some of the instance variables used to just pass fixed
    // arguments to the OpenCL kernel.
    const int agentCount     = oclAgents.size();
    const int hostStateSize  =
        static_cast<HCState*>(oclAgents[0]->getState())->getHCStateSize();
    const int hostParamsSize = oclAgents[0]->getHCParamSize();
    const muse::Time gvt     = getGVT();
    const int endianCheck    = 0xaabbcc;
    oclKernel.setArg(0, agentCount);
    oclKernel.setArg(1, agentIDsBuf);
    oclKernel.setArg(2, paramBuf);
    oclKernel.setArg(3, hostParamsSize);
    oclKernel.setArg(4, stateBuf);
    oclKernel.setArg(5, hostStateSize);
    oclKernel.setArg(6, oclRunLGVT);
    oclKernel.setArg(7, gvt);
    oclKernel.setArg(8, rndGenBuf);
    oclKernel.setArg(9, endianCheck);
    // Schedule the openCL kernel to run.
    isKernelRunning = true;
    try { 
        queue.enqueueNDRangeKernel(oclKernel, cl::NullRange,
                                   cl::NDRange(oclAgents.size()), cl::NullRange,
                                   NULL, &oclRunEvent);
    } catch (cl::Error error) {
        std::cerr << error.what() << "(" << error.err() << ")" << std::endl;
        throw error;
    }
    // Clear out oclAgents to start collecting next set of agents
    oclAgents.clear();
    // Wait only in blocking mode. If not other operations can proceed.
    if (blocking) {
        waitForOclKernel();  // Block and wait for schedule to finish
    }
}


void
OclSimulation::waitForOclKernel() {
    if (isKernelRunning) {
        // Wait for currently running kernel to finish.
        oclRunEvent.wait();
        // Track the time it took to run the kernel (in nanoseconds)
        const cl_ulong startTime =
            oclRunEvent.getProfilingInfo<CL_PROFILING_COMMAND_START>();
        const cl_ulong endTime =
            oclRunEvent.getProfilingInfo<CL_PROFILING_COMMAND_END>();
        oclRunTime += (endTime - startTime);
        // Copy the state back to the respective agents if no
        // rollbacks.
        copyDataFromDevice();
        // Reset running flag for the OCL kernel.
        isKernelRunning = false;        
    }
}

void
OclSimulation::copyDataFromDevice() {
    if ((LGVT < oclRunLGVT) || (lastLGVT != oclRunLGVT) ||
        runningAgents.empty()) {
        return;  // Rollback has occurred. Do not copy info back.
    }
    // Use OCL buffer manager (ease mapping/unmapping). We never
    // resize during reads.
    OclBufferManager stateBufMgr(stateBuf, queue);
    char* stateBuf = stateBufMgr.map<char>(false);  // false == read
    // Now copy the state information to the device for each agent
    // that has finished running.  NOTE: We use intentionally use the
    // runningAgents list and not the oclAgents
    const int stateSize = static_cast<muse::HCState*>(runningAgents[0]->
                                          getState())->getHCStateSize();
    for (size_t i = 0, bufPos = 0; (i < runningAgents.size());
         i++, bufPos += stateSize) {
        // Shortcut to agent and its state
        muse::HCAgent* agent = oclAgents[i];
        ASSERT(dynamic_cast<muse::HCState*>(agent->getState()) != NULL);
        muse::HCState* state = static_cast<muse::HCState*>(agent->getState());
        // Copy the state to the buffer. All states currently should
        // be of the same size.
        ASSERT( state->getHCStateSize() == stateSize);
        state->copyFromDevice(stateBuf + bufPos, stateSize);
    }
}

void
OclSimulation::garbageCollect() {
    // First wait for all currently scheduled OCL kernels (if any) to
    // finish running.
    waitForOclKernel();
    // Now trigger garbage collection
    Simulation::garbageCollect();
}

// Include the MuseOclKernel.c OpenCL kernel interface codefor use
// during kernel generation.
const std::string OclSimulation::MuseOclKernel =
#include "ocl/MuseOclKernel.c"
;  // Trailing semicolon important!

// Include the MuseOclLibrary.ocl OpenCL kernel library for use during
// kernel generation.
const std::string OclSimulation::MuseOclLibrary =
#include "ocl/MuseOclLibrary.c_inc"
;  // Trailing semicolon important!

END_NAMESPACE(muse);

#endif
