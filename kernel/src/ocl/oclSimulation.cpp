#ifndef MUSE_OCLSIMULATION_CPP
#define MUSE_OCLSIMULATION_CPP
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
// Authors: Harrison Roth          rothhl@miamioh.edu
//          Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include "ocl/oclSimulation.h"
#include <ctime>
#include "MPIHelper.h"
#include "Communicator.h"
#include <vector>
#include <string>
#include <algorithm>

BEGIN_NAMESPACE(muse);

oclSimulation::oclSimulation(const bool useSharedEvents)
    : Simulation(useSharedEvents) {
    commManager        = NULL;
    scheduler          = NULL;
    myID               = -1u;
    listener           = NULL;
    doDumpStats        = false;
    mustSaveState      = false;
    maxMpiMsgThresh    = 1000;
    processMpiMsgCalls = 0;
    mpiMsgCheckThresh  = 1;
    mpiMsgCheckCounter = mpiMsgCheckThresh;
    DEBUG(logFile      = NULL);
    lastLGVT           = 0;
}

bool
oclSimulation::processNextEvent() {
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
    // call returns false.
    // Pass null agent, if it returns with value, add agent to list of
    // agents to be bulk processed by gpu
    AgentID ret = kernel->scheduler->processNextAgentEvents();

    if (ret != InvalidOCLAgentID && ret != InvalidAgentID) {
        // add agent to list of agents that need equations run
        Agent* agent = kernel->allAgents[ret];
        oclAgents.push_back(reinterpret_cast<oclAgent*>(agent));
    }
    return ret;
}

cl::Platform
oclSimulation::getPlatform() {
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);

    if (all_platforms.size() == 0) {
        std::cout << "No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    return all_platforms[0];
}

cl::Device
oclSimulation::getDevice(cl::Platform platform, int i, bool display) {
    std::vector<cl::Device> all_devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if (all_devices.size() == 0) {
        std::cout << "No devices found. Check OpenCL installation!\n";
        exit(1);
    }

    if (display) {
        for (unsigned int j=0; j < all_devices.size(); j++)
            printf("Device %d: %s\n", j, all_devices[j].getInfo<CL_DEVICE_NAME>().c_str());
    }
    return all_devices[i];
}

void
oclSimulation::initOCL(oclAgent* agent, int maxGlobal) {
    // initialize opencl platform and devices
    cl::Platform plat = getPlatform();
    cl::Device dev     = getDevice(plat, 0, false);
    cl::Context ctx(dev);
    cl::Program::Sources src;

    std::string kernel_code = agent->getKernel();
    src.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program prog(ctx, src);
    if (prog.build({dev}) != CL_SUCCESS) {
        std::cout << "Error building: " << prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev) << std::endl;
        exit(1);
    }

    cl::CommandQueue q(ctx, dev);
    cl::Kernel r = cl::Kernel(prog, "run");
    cl::Buffer buffer_stp(ctx, CL_MEM_READ_WRITE, sizeof(float));
    float stp[1] = {step};
    q.enqueueWriteBuffer(buffer_stp, CL_TRUE, 0, sizeof(float), stp);
    cl::Buffer buffer_comp(ctx, CL_MEM_READ_WRITE, sizeof(int));
    int comp[1] = {compartments};
    q.enqueueWriteBuffer(buffer_comp, CL_TRUE, 0, sizeof(int), comp);
    cl::Buffer B(ctx, CL_MEM_READ_WRITE, sizeof(real) * (maxGlobal));

    // Initialize random values for ssa version
    int numRand = 100;
    cl::Buffer buffer_rnd(ctx, CL_MEM_READ_WRITE, sizeof(real) * (numRand));
    srand(time(NULL));
    if (!ode) {
        std::vector<real> randomValues;
        for (int i = 0; i < numRand; i++) {
            randomValues.push_back(static_cast <real> (rand()) / static_cast <real> (RAND_MAX));
        }
        q.enqueueWriteBuffer(buffer_rnd, CL_TRUE, 0, sizeof(real)*(numRand), randomValues.data());
    }

    queue = q;
    run = r;
    buffer_B = B;
    buffer_random = buffer_rnd;
    buffer_compartments = buffer_comp;
}

void
oclSimulation::parseClassVars(int& argc, char* argv[])
    throw(std::exception) {
    bool ocl = false;
    float stp = 0.001f;
    int comp = 4;
    bool ssa = false;
    int maxWG = CL_DEVICE_MAX_WORK_GROUP_SIZE;
    ArgParser::ArgRecord arg_list[] = {
        { "--ocl", "run OpenCL version of simulation",
          &ocl, ArgParser::BOOLEAN},
        { "--ssa", "run stochastic version of simulation",
          &ssa, ArgParser::BOOLEAN},
        { "--step", "Step size for epidemic equations",
          &stp, ArgParser::FLOAT},
        { "--compartments", "Number of compartments in simulation",
          &comp, ArgParser::INTEGER},
        { "--workgroup", "Number of agents run on GPU at a time",
          &maxWG, ArgParser::INTEGER},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the MUSE argument parser to parse command-line arguments
    // and update instance variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    oclAvailable = ocl;
    step = stp;
    compartments = comp;
    ode = !ssa;
    maxWorkGroups = maxWG;
}

void
oclSimulation::start() {
    // Finish all the setup prior to starting simulation.
    preStartInit();
    // If no agents registered
    // we need to leave start and end sim
    if (allAgents.empty()) return;
    // Next initialize all the agents
    initAgents();
    // Set up OpenCL if needed
    int maxGlobal;
    if (oclAvailable) {
        std::cout <<  "Initialize ocl" << std::endl;
        // determine maximum number of agents to send to the GPU for processing
        maxGlobal = std::min(maxWorkGroups, (int)allAgents.size());
        initOCL((oclAgent*)kernel->allAgents[0], maxGlobal);
    }
    // Start the core simulation loop.
    LGVT         = startTime;
    lastLGVT = startTime;
    int gvtTimer = 2;
    // The main simulation loop
    while (gvtManager->getGVT() < endTime) {
        // See if a stat dump has been requested
        if (doDumpStats) {
            dumpStats();
            doDumpStats = false;
        }

        if (--gvtTimer == 0) {
            gvtTimer = gvtDelayRate;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
        // Process a block of events received via the network, while
        // performing exponential backoff as necessary.
        checkProcessMpiMsgs();
        // Process the next event from the list of events managed by
        // the scheduler.

        LGVT = kernel->scheduler->getNextEventTime();
        if (oclAvailable && ((LGVT > lastLGVT && !oclAgents.empty())
                || oclAgents.size() > maxGlobal)) {
            lastLGVT = LGVT;
            processAgents(maxGlobal);
        }
        
        if (processNextEvent() == InvalidAgentID) {
            // We did not have any events to process. So check MPI
            // more frequently.
            mpiMsgCheckCounter = 1;
        }
    }
    MPI_BARRIER();
    finalizeSimulation();
}

void
oclSimulation::initialize(int& argc, char* argv[], bool initMPI)
    throw(std::exception) {
    commManager = new Communicator();
    myID = commManager->initialize(argc, argv, initMPI);
    unsigned int numThreads;  // dummy. not really used.
    commManager->getProcessInfo(myID, numberOfProcesses, numThreads);
    // Consume any specific command-line arguments used to setup and
    // configure other components like the scheduler and GVT manager.
    parseCommandLineArgs(argc, argv);    
    // Finally, let the base-class perform generic initialization
    muse::Simulation::initialize(argc, argv, initMPI);
    parseClassVars(argc, argv);
}

void
oclSimulation::preStartInit() {
    Simulation::preStartInit();
    // Next, we setup/finalize the AgentMap for all kernels
    commManager->registerAgents(allAgents);
}

void
oclSimulation::processAgents(int maxGlobal) {
    // parse data from agents
    int count = oclAgents.size();
    int c = 0;
    std::vector<real> values;
    for (oclAgent* agent : oclAgents) {
        for (int i = 0; i < compartments; i++) {
            values.push_back(agent->myState->values[i]);      
            std::cout << values[i+c] << ", ";
        }
        std::cout << std::endl;
        c+=compartments;
    }
    real* vals = values.data();
    // copy data onto GPU
    cl_int status = queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, sizeof(real)*(maxGlobal), vals);
    if (status != 0) {
        std::cout << "OpenCL Status Error: " << status << std::endl;
    }
    // Set args for kernel code
    status = run.setArg(0, buffer_B);
    if (!ode) {
        run.setArg(1, buffer_random);
    }
    // run kernel code on agent data
    // the last input into enqueueNDRangeKernel can specify the local work size, but leaving it null allows the opencl to determine the local work size
    queue.flush();
    status = queue.enqueueNDRangeKernel(run, cl::NullRange, cl::NDRange(count), cl::NullRange);
    if (status != 0) {
        std::cout << "OpenCL Status Error: " << status << std::endl;
    }    queue.flush();
    // read data back to values array
    status = queue.enqueueReadBuffer(buffer_B, CL_TRUE, 0, sizeof(real)*(maxGlobal), vals);
    if (status != 0) {
        std::cout << "OpenCL Status Error: " << status << std::endl;
    }
    queue.flush();

    // put altered data back into agents        
    c = 0;
    for (oclAgent* agent : oclAgents) {
       for (int i = 0; i < compartments; i++) {
            agent->myState->values[i] = vals[i+c];
            std::cout << vals[i+c] << ", ";
        }
       std::cout <<std::endl;
       c+=compartments;
    }
    // clear list of agent ids for ocl
    oclAgents.clear();
}

END_NAMESPACE(muse);

#endif
