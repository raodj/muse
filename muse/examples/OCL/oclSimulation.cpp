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

#include "oclSimulation.h"
#include "MPIHelper.h"
#include "kernel/include/Communicator.h"
#include <ctime>
#include "synthAgent.h"

BEGIN_NAMESPACE(muse);

// Define reference to the singleton simulator/kernel
muse::oclSimulation* muse::oclSimulation::kernel = NULL;

oclSimulation::oclSimulation(const bool useSharedEvents)
    : Simulation(useSharedEvents){
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

bool oclSimulation::processNextEvent() { 
    // Only run when not using OCL
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

    oclAgent* agent = NULL;
    // Let the scheduler do its task to have the agent process events
    // if possible.  If no events are processed the following method
    // call returns false.
    bool ret = scheduler->processNextAgentEvents(&agent);
    return ret;
}

void oclSimulation::start(){
    // If no agents registered
    // we need to leave start and end sim
    if (allAgents.empty()) return;
    // Next initialize all the agents
    initAgents();
    // Set up OpenCL device if needed
    if(kernel->oclAvailable) {
        oclRun();
    }else{
        run();
    }
    // Wait for all the parallel processes to complete the main
    // simulation loop.
    MPI_BARRIER();
}

Platform oclSimulation::getPlatform() {
    std::vector<Platform> all_platforms;
    Platform::get(&all_platforms);

    if (all_platforms.size()==0) {
        cout << "No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
//     for(unsigned int i = 0; i < all_platforms.size(); i++)
//         cout << "Using platform: "<< all_platforms[i].getInfo<CL_PLATFORM_NAME>() <<"\n";
    return all_platforms[0];
}


Device oclSimulation::getDevice(Platform platform, int i, bool display) {
    std::vector<Device> all_devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if(all_devices.size()==0){
        cout << "No devices found. Check OpenCL installation!\n";
        exit(1);
    }

    if (display) {
        for (unsigned int j=0; j<all_devices.size(); j++)
            printf("Device %d: %s\n", j, all_devices[j].getInfo<CL_DEVICE_NAME>().c_str());
    }
//     for (unsigned int j=0; j<all_devices.size(); j++){
//         cout << "Device " << i << " : " << all_devices[i].getInfo<CL_DEVICE_TYPE>() << endl;
//         cout << "Device " << j << " : " << all_devices[j].getInfo<CL_DEVICE_EXECUTION_CAPABILITIES>() << endl;
//         cout << "Device " << j << " : " << all_devices[j].getInfo< CL_DEVICE_VERSION >() << endl;
//         cout << "Device " << i << " : " << all_devices[i].getInfo< CL_DEVICE_NAME >() << endl;
//     }
    return all_devices[i];
}

std::string oclSimulation::getKernel(){
    // check if using ssa or ode kernel
    // then kernel string is returned
    if(!ode){
        std::string s = std::to_string(kernel->step);
        const char *pchar = s.c_str();
        s = std::to_string(compartments);
        const char *comp = s.c_str();
        float rate = .0001f;
        s = std::to_string(rate);
        const char *r = s.c_str();
        std::string ssa_kernel_code = "void kernel run(global float* cv, global float* random) {\n"
        "    int compartments = ";
        ssa_kernel_code.append(comp);
        ssa_kernel_code.append(";\n"
        "    int id = get_global_id(0)*compartments;\n"
        "    float stp =");
        ssa_kernel_code.append(pchar);
        ssa_kernel_code.append("f;\n"
        "    const float rate = "); 
        ssa_kernel_code.append(r);
        ssa_kernel_code.append("f;\n"
        "    const float EventChanges[");
        ssa_kernel_code.append(comp);
        ssa_kernel_code.append("+1][");
        ssa_kernel_code.append(comp);
        ssa_kernel_code.append("] = {\n");
        
        for(int i = 0; i < compartments-1; i++){
            ssa_kernel_code.append("{");
            for(int j = 0; j < compartments; j++){
                if(j!=0){
                    ssa_kernel_code.append(", ");
                }
                if(i==j){
                    ssa_kernel_code.append("-1");
                }
                else if(j-i == 1){
                    ssa_kernel_code.append("1");
                }else{
                    ssa_kernel_code.append("0");
                }
            }
            if(i+1 == compartments-1){
                ssa_kernel_code.append("}\n");
            }else{
                ssa_kernel_code.append("},\n");
            }  
        }
        ssa_kernel_code.append("    };\n"
        "   for(int x = 0; x < (int)1/stp; x++){\n"
        "     for(int i = 0; (i < (compartments-1)); i++) {\n"
        "        for(int j = 0; j < compartments; j++){\n"
        "           float scale = rate*random[(id+x)%100]*stp;\n"
//        "           cv[j+id] = cv[j+id] + (EventChanges[i][j] * scale);\n"
        "           cv[j+id] = cv[j+id] + scale;\n"
        "        }\n"
        "     }\n"
        "   }\n"
        "}\n");
        return ssa_kernel_code;
    }else{
        std::string s = std::to_string(compartments);
        const char *pchar = s.c_str();
        s = std::to_string(kernel->step);
        const char *h = s.c_str();
        std::string ode_kernel_code = "void kernel run(global float* xl) {\n"
        "   int compartments = ";
        ode_kernel_code.append(pchar);
        ode_kernel_code.append(";\n"
        "   float h = ");
        ode_kernel_code.append(h);
        ode_kernel_code.append("f;\n"
        "   float A = (float)0.0005f;\n"
        "   float B  = (float)0.00004f;\n"
        "   int id = get_global_id(0)*compartments;\n"
        "  \n"
        "        // Use runge-kutta 4th order method here.\n"
        "   float k1[");
        ode_kernel_code.append(pchar);
        ode_kernel_code.append("], k2[");
        ode_kernel_code.append(pchar);
        ode_kernel_code.append("], k3[");
         ode_kernel_code.append(pchar);
        ode_kernel_code.append("], k4[");
         ode_kernel_code.append(pchar);
        ode_kernel_code.append("], xlt[");
         ode_kernel_code.append(pchar);
        ode_kernel_code.append("];\n");
         ode_kernel_code.append("        // Compute k1\n"
        "   for(int i = 0; i < (int)1/h; i++){\n"
            "   for(int i = 1; i < compartments; i++){\n"
            "       k1[i] = (xl[i-1] * A) - (B * xl[i]);\n"
            "    }\n"
            "    k1[0] = -k1[1];\n"
                  
            "    for(int i = 0; i < compartments; i++){\n"
            "        xlt[i] = xl[i] + k1[i] * h / 2;\n"
            "    }\n"
   
            "   for(int i = 1; i < compartments; i++){\n"
            "       k2[i] = (xlt[i-1] * A) - (B * xlt[i]);\n"
            "    }\n"
            "    k2[0] = -k2[1];\n"

            "    for(int i = 0; i < compartments; i++){\n"
            "        xlt[i] = xl[i] + k2[i] * h/ 2;\n"
            "    }\n"                

            "   for(int i = 1; i < compartments; i++){\n"
            "       k3[i] = (xlt[i-1] * A) - (B * xlt[i]);\n"
            "    }\n"
            "    k3[0] = -k3[1];\n"                

            "    for(int i = 0; i < compartments; i++){\n"
            "        xlt[i] = xl[i] + k3[i] * h;\n"
            "    }\n"

            "   for(int i = 1; i < compartments; i++){\n"
            "       k4[i] = (xlt[i-1] * A) - (B * xlt[i]);\n"
            "    }\n"
            "    k4[0] = -k4[1];\n"                
            "\n"

            "    for(int i = 0; i < compartments; i++){\n"
            "        xl[i] = xl[i] + (k1[i] + 2*k2[i] + 2*k3[i] + k4[i]) * h / 6;\n"
            "    }\n"        
                
        "    }  \n"
        "}\n");
        return ode_kernel_code;
    }
}

void
oclSimulation::createAgents() {
    const int max_nodes      = getNumberOfProcesses();
    const int threadsPerNode = getNumberOfThreads();
    const int skewAgents     = 0;
    const int agentsPerNode  = ((rows*cols) - skewAgents) / max_nodes;
    ASSERT( agentsPerNode > 0 );    
    const int agentsPerThread= std::max(1, agentsPerNode / threadsPerNode);
    ASSERT( agentsPerThread > 0 );
    const int agentStartID   = 0;
    const int agentEndID     = rows * cols;   
    // Create the agents and assign to multiple threads if that is the
    // configuration.
    int currThread       = 0;  // current thread
    int currThrNumAgents = 0;  // number of agents on current thread.
    for (int i = agentStartID; (i < agentEndID); i++) {
        oclState* state = new oclState(compartments, popSize, expSize);
//        cout << popSize << " , "<< expSize << ", " << compartments << endl;
//        for(int i = 0; i < compartments; i++){
//            cout << "State Value " << i << ": " << state->values[i] <<endl;
//        }
        oclAgent* agent = new synthAgent(i, state, kernel->oclAvailable, kernel->step, compartments, kernel->ode);
//        oclAgent* agent = new oclAgent(i, state, kernel->oclAvailable, kernel->step, compartments, kernel->ode);
        kernel->registerAgent(agent, currThread);
        agent->initialize();


        // Handle assigning agents to different threads
        if ((++currThrNumAgents >= agentsPerThread) &&
            (currThread < threadsPerNode - 1)) {
            currThread++;          // assign agents to next thread.
            currThrNumAgents = 0;  // reset number of agents on this thread.
        }

    }
//    std::cout << "Registered agents from "
//              << agentStartID    << " to "
//              << agentEndID      << " agents.\n";
}

oclSimulation*
oclSimulation::initializeSimulation(int& argc, char* argv[], bool initMPI)
    throw (std::exception) {
    // First use a temporary argument parser to determine type of
    // simulation kernel to instantiate.
    // Take arguments for row and col to determine number of agents
    // Stop time to determine sim length
    // population and exposed per agent initailly
    // step size for epidemic equations
    std::string simName = "default";
    int row = 1;
    int col = 1;
    bool ocl = false;
    int stop = 2;
    int pop = 25000;
    int exp = 30;
    float stp = 0.001f;
    int comp = 4;
    bool ssa = false;
    int maxWG = 0;
    ArgParser::ArgRecord arg_list[] = {
        { "--row", "number of rows in simulation, dictates number of agents",
          &row, ArgParser::INTEGER},
        { "--col", "number of columns in simulation, dictates number of agents",
          &col, ArgParser::INTEGER},
        { "--ocl", "run OpenCL version of simulation",
          &ocl, ArgParser::BOOLEAN},
        { "--ssa", "run stochastic version of simulation",
          &ssa, ArgParser::BOOLEAN},
        { "--endtime", "Number of time steps in simulation",
          &stop, ArgParser::INTEGER},
        { "--pop", "Number of people initially in each agent",
          &pop, ArgParser::INTEGER},
        { "--exp", "Number of people exposed initially in each agent",
          &exp, ArgParser::INTEGER},
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
    kernel = new oclSimulation();
    // Now let the instantiated/derived kernel initialize further.
    ASSERT (kernel != NULL);
    kernel->initialize(argc, argv, initMPI);  // can throw exception!
    // When control drops here, things went fine thus far
    kernel->scheduler = new oclScheduler();
    kernel->scheduler->initialize(myID, numberOfProcesses, argc, argv);
    kernel->commManager = new Communicator();
    kernel->commManager->initialize(argc, argv, initMPI);
    rows = row;
    cols = col;
    kernel->oclAvailable = ocl;
    stopTime = stop;
    popSize = pop;
    expSize = exp;
    kernel->step = stp;
    kernel->compartments = comp;
    kernel->ode = !ssa;
    kernel->maxWorkGroups = maxWG;
    return kernel;
}

void
oclSimulation::simulate() {
    // Convenient local reference to simulation kernel
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(stopTime);
    
    // Finish all the setup prior to starting simulation.
    kernel->preStartInit();
//    cout << "Create Agents" << endl;
    //Create Agents for simulation after start and stop times are set
    createAgents();
//    cout << "Created Agents" << endl;
//     Finally start the simulation here!!
    kernel->start();
//    cout << "Finished Simulation Loop" << endl;
    // Now we finalize the simulation to make sure it cleans up.
    finalizeSimulation();
}

bool
oclSimulation::registerAgent(oclAgent* agent, const int threadRank)  {
    UNUSED_PARAM(threadRank);
    if (scheduler->addAgentToScheduler(agent)) {
        allAgents.push_back(agent);
        agent->mustSaveState = this->mustSaveState;
        agent->setKernel(this);
        return true;
    }
    return false;
}

bool 
oclSimulation::scheduleEvent(Event* e) {
    ASSERT(e->getReceiveTime() >= getGVT());
    ASSERT(e->getReferenceCount() == 1);
    
    if (TIME_EQUALS(e->getSentTime(), TIME_INFINITY) ||
        (e->getSenderAgentID() == -1)) {
        std::cerr << "Don't use this method with a new event, go "
                  << "through the agent's scheduleEvent method." << std::endl;
        abort();
    }
    const AgentID recvAgentID = e->getReceiverAgentID();
    
//    if (isAgentLocal(recvAgentID)) {
        // Local events are directly inserted into our own scheduler
        return scheduler->scheduleEvent(e);
//    } else {
        // Remote events are sent via the GVTManager to aid tracking
        // GVT. The gvt manager calls communicator.
//        kernel->gvtManager->sendRemoteEvent(e);
//    }
    return true;
}

bool
oclSimulation::isAgentLocal(AgentID id) const {
    return commManager->isAgentLocal(id);
}

void
oclSimulation::preStartInit() {
	// Note: In share memory multi threaded sims, this method is only 
	// called by the manager thread as the gvtManager is then shared 
	// directly with the other threads
    ASSERT( commManager != NULL );
	// Ensure we don't populate this in a derived class and accidently
	// overwrite it here
    ASSERT(gvtManager == NULL);
    // Create and initialize our GVT manager.
    gvtManager = new GVTManager(this);
    gvtManager->initialize(startTime, commManager);
    // Set gvt manager with the communicator.
    commManager->setGVTManager(gvtManager);
    // Inform scheduler(s) that simulation is starting.
    scheduler->start(startTime);
}

void
oclSimulation::garbageCollect() {
    const Time gvt = getGVT();
    // First let the scheduler know it can garbage collect.
    scheduler->garbageCollect(gvt);
    for (AgentContainer::iterator it = allAgents.begin();
         (it != allAgents.end()); it++) {  
        (*it)->garbageCollect(gvt);
    }
}

void
oclSimulation::finalizeSimulation(bool stopMPI, bool delSim) {
    // First let the derived class finalize
    ASSERT( kernel != NULL );
//    kernel->finalize(stopMPI);
    // If the simulator is to be deleted, do that now.
    if (delSim) {
        delete kernel;  // get rid of singleton instance.
        kernel = NULL;
    }
}

//void
//oclSimulation::finalize(bool stopMPI, bool delCommMgr) {
//    // Inform the scheduler that the simulation is complete
//    scheduler->stop();
//    // Finalize all the agents on this MPI process while accumulating stats
//    for (AgentContainer::iterator it = allAgents.begin();
//	 it != allAgents.end(); it++) {
//	Agent* const agent = *it;
//        agent->finalize();
//        agent->garbageCollect(TIME_INFINITY);
//        agent->cleanStateQueue();
//        agent->cleanInputQueue();
//        // Don't clean output queue yet as we need stats from it.
//    }
//    // Report aggregate statistics from this kernel
////    reportStatistics();
//
//    // Clean up all the agents
//    for (AgentContainer::iterator it = allAgents.begin();
//	 it != allAgents.end(); it++) {
//        Agent* const agent = *it;
//        agent->cleanOutputQueue();
//        // Remove agent from scheduler
//        scheduler->removeAgentFromScheduler(agent);
//        // Bye byte agent!
////        delete agent;
//    }
//
//    // Now delete GVT manager as we no longer need it.
//    commManager->setGVTManager(NULL);
//    delete gvtManager;
//    gvtManager = NULL;
//    
//    // Finalize the communicator 
////    commManager->finalize(stopMPI);
//    // Delete it if requested
//    if (delCommMgr) {
//        delete commManager;
//    }
//    commManager = NULL;
//
//    // Invalidate the  kernel ID
//    myID = -1u;
//
//    DEBUG({
//            if (logFile != NULL) {
//                // Un-hijack cout
//                std::cout.rdbuf(oldstream);
//                // Get rid of the log file.
//                delete logFile;
//                logFile = NULL;
//            }
//    });
//    // Get rid of scheduler as we no longer need it
//    delete scheduler;
//    scheduler = NULL;
//    // Clear out the list of agents in this simulation
//    allAgents.clear();
//    // Finally clear out any pending events in the event recyler.
//    EventRecycler::deleteRecycledEvents();
//    // Clear out any pending states in the event recycler
////    StateRecycler::deleteRecycledStates();
//}

void oclSimulation::oclRun(){
    // initialize opencl platform and devices 
    Platform default_platform = getPlatform();
    Device default_device     = getDevice(default_platform, 0, false);
    Context context(default_device); 
    Program::Sources sources;

    // set opencl kernel
    std::string kernel_code = getKernel();
    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    Program program(context, sources);
    if (program.build({default_device}) != CL_SUCCESS) {
        cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
        exit(1);
    }

    CommandQueue queue(context, default_device);
    Kernel run = Kernel(program, "run");
    // determine maximum number of agents to send to the GPU for processing
    int maxGlobal = std::min(CL_DEVICE_MAX_WORK_GROUP_SIZE*100, rows*cols*compartments);
    // if user specified number of agents, use that value if less than the current max
    if(kernel->maxWorkGroups != 0){
        maxGlobal = std::min(kernel->maxWorkGroups*compartments, maxGlobal);
    }
    // initialize array to hold agent values to be passed onto GPU
    real values[maxGlobal];
    
    Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(real) * (maxGlobal));
    // Initialize random values for ssa version
    int numRand = 100;
    Buffer buffer_random(context, CL_MEM_READ_WRITE, sizeof(real) * (numRand));
    srand ( time(NULL) );
    if(!ode){
        float randomValues[numRand];
        for(int i = 0; i < numRand; i++){
            randomValues[i] = static_cast <real> (rand()) / static_cast <real> (RAND_MAX);
        }
        queue.enqueueWriteBuffer(buffer_random, CL_TRUE, 0, sizeof(real)*(numRand), randomValues);
    }
    
    // Start the core simulation loop.
    LGVT         = startTime;
    lastLGVT = startTime;
    int gvtTimer = gvtDelayRate;
    // The main simulation loop
    while (gvtManager->getGVT() < endTime) {
        // See if a stat dump has been requested
        if (doDumpStats) {
            dumpStats();
            doDumpStats = false;
        }
        
        if (--gvtTimer == 0 ) {
            gvtTimer = gvtDelayRate;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
        // Process a block of events received via the network, while
        // performing exponential backoff as necessary.
        checkProcessMpiMsgs();
        // Process the next event from the list of events managed by
        // the scheduler.

        // this is from process next event --> all opencl code was moved to the
        // start function in order to fix any scoping issues and allow the opencl
        // initialization to occur only once.
            kernel->LGVT = scheduler->getNextEventTime();
            if((kernel->LGVT > lastLGVT && oclAvailable && !oclAgents.empty()) || oclAgents.size() >= maxGlobal/compartments){
//                std::cout<<"---Doing Bulk Processing at "<< lastLGVT  << ", " << oclAgents.size()<< " ---" << std::endl;

                lastLGVT = kernel->LGVT;        

                // parse data from agents
                int count = oclAgents.size();
                int c = 0;
                for(oclAgent* agent : oclAgents){
                    for(int i = 0; i < compartments; i++){
                        values[i+c] = agent->myState->values[i];      
                    }
                    c+=compartments;
                }

                // Run OCL kernel on agent data from list
                // copy data onto GPU
                cl_int wstatus = queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, sizeof(real)*(maxGlobal), values);
//                std::cout << "Write Status: " << wstatus << std::endl;
                // Set args for kernel code
                cl_int astatus = run.setArg(0, buffer_B);
                if(!ode){
                    run.setArg(1, buffer_random);
                }
                
//                std::cout << "Set Arg Status: " << astatus << std::endl;
                // run kernel code on agent data
                // the last input into enqueueNDRangeKernel can specify the local work size, but leaving it null allows the opencl to determine the local work size
                queue.flush();
                cl_int status = queue.enqueueNDRangeKernel(run, NullRange, NDRange(count),NullRange);
                queue.flush();
//                std::cout << "Status: " << status << std::endl;
                // read data back to values array
                status = queue.enqueueReadBuffer(buffer_B, CL_TRUE, 0, sizeof(real)*(maxGlobal), values);
//                std::cout << "Read Status: " << status << std::endl;
                queue.flush();
                
                // put altered data back into agents        
                c = 0;
//                bool first = true;
                for(oclAgent* agent : oclAgents){
                   for(int i = 0; i < compartments; i++){
                        agent->myState->values[i] = values[i+c];
//                        if(first){
//                            std::cout << values[i+c] << ", ";
//                        }
                    }
//                   if(first){
//                    std::cout << "\n";
//                   }
//                   first = false;
                   c+=compartments;
                }
                // clear list of agent ids for ocl
                oclAgents.clear();
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
            oclAgent* agent = NULL;
            // Let the scheduler do its task to have the agent process events
            // if possible.  If no events are processed the following method
            // call returns false.
            // Pass null agent, if it returns with value, add agent to list of 
            // agents to be bulk processed by gpu
            bool ret = scheduler->processNextAgentEvents(&agent);

            if(agent != NULL){
                //add agent to list of agents that need equations run
                oclAgents.push_back(agent);
            }
        if(!ret){
            // We did not have any events to process. So check MPI
            // more frequently.
            mpiMsgCheckCounter = 1;
        }
    }
    queue.finish();
}


void oclSimulation::run(){
    // Start the core simulation loop.
    LGVT         = startTime;
    lastLGVT = startTime;
    int gvtTimer = gvtDelayRate;
    // The main simulation loop
    while (gvtManager->getGVT() < endTime) {
//        cout << "start loop " << endl;
        // See if a stat dump has been requested
        if (doDumpStats) {
            dumpStats();
            doDumpStats = false;
        }
        if (--gvtTimer == 0 ) {
            gvtTimer = gvtDelayRate;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
        // Process a block of events received via the network, while
        // performing exponential backoff as necessary.
        checkProcessMpiMsgs();
        kernel->LGVT = scheduler->getNextEventTime();
        // Process the next event from the list of events managed by
        // the scheduler.
        if (!processNextEvent()) {
            // We did not have any events to process. So check MPI
            // more frequently.
            mpiMsgCheckCounter = 1;
        }
    }
}
END_NAMESPACE(muse);

int 
main(int argc, char** argv) {
    muse::oclSimulation* sim = new muse::oclSimulation();
//    std::cout << "start" << std::endl;
    sim->initializeSimulation(argc, argv, true);
//    std::cout << "Initialized Simulation" << std::endl;
    sim->simulate();
    return 0;
}

#endif