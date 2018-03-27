#ifndef MUSE_OCLSIMULATION_CPP
#define MUSE_OCLSIMULATION_CPP
/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "oclSimulation.h"
#include "MPIHelper.h"
#include "kernel/include/Communicator.h"
BEGIN_NAMESPACE(muse);

std::vector<OCLAgent*> oclAgents;
CommandQueue queue;
Kernel run;
Context context;
Buffer buffer_A;
bool ssa = false;
bool oclAvailable = false;
float step = 1.0f;
int compartments = 4;
float* values;
Time lastLGVT;
int rows = 100;
int cols = 150;
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
    //when lgvt is increased -> run ocl kernel on agents that need it 
    //      --> get agents that need ocl code run
    // Update lgvt to the time of the next event to be processed.
    kernel->LGVT = scheduler->getNextEventTime();
    if(kernel->LGVT > lastLGVT && oclAvailable && !oclAgents.empty()){
        std::cout<<"---Doing Bulk Processing at "<< lastLGVT  << ", " << oclAgents.size()<< " ---" << std::endl;
        lastLGVT = kernel->LGVT;
        //parse data from agents
        int count = oclAgents.size();
        int c = 0;
        float values[compartments*oclAgents.size()];
        for(OCLAgent* agent : oclAgents){
            values[c] = agent->myState->susceptible;
            values[c+1] = agent->myState->exposed;
            values[c+2] = agent->myState->infected;
            values[c+3] = agent->myState->recovered;
            
            c++;
            break;
        }
        //Run OCL kernel on agent data from list
//        if(ssa){
//            int rnd[1] = {0};
//            float stp[1] = {step};
//            Buffer buffer_step(context, CL_MEM_READ_WRITE, sizeof(float));
//            Buffer buffer_rnd(context, CL_MEM_READ_WRITE, sizeof(int));
//            queue.enqueueWriteBuffer(buffer_step, CL_TRUE, 0, sizeof(float), stp);
//            queue.enqueueWriteBuffer(buffer_rnd, CL_TRUE, 0, sizeof(int), rnd);
//        }
        Buffer buffer_B(context, CL_MEM_READ_WRITE, sizeof(float) * (count));
        queue.enqueueWriteBuffer(buffer_B, CL_TRUE, 0, sizeof(float)*(count), values);
        
//        if(!ssa){
            run.setArg(0, buffer_B);
//        }else{
//            run.setArg(0, buffer_B);
//            run.setArg(1, buffer_A);
//            run.setArg(2, buffer_rnd);
//            run.setArg(3, buffer_step);
//        }
        queue.enqueueNDRangeKernel(run, 0, count/4, 100);
        queue.enqueueReadBuffer(buffer_B, CL_TRUE, 0, sizeof(float)*(count), values);
        queue.finish();
        //put altered data back into agents
        c = 0;
        for(OCLAgent* agent : oclAgents){
            agent->myState->susceptible = values[c];
            agent->myState->exposed = values[c+1];
            agent->myState->infected = values[c+2];
            agent->myState->recovered = values[c+3];
            
            c++;
        }
        //clear list of agent ids for ocl
        oclAgents.clear();
    }//else{
//        std::cout << "Next event within time step " << kernel->LGVT  << ", " << lastLGVT << std::endl; 
//    }
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

    OCLAgent* agent = NULL;
    // Let the scheduler do its task to have the agent process events
    // if possible.  If no events are processed the following method
    // call returns false.
    //Pass null agent, if it returns with value, add agent to list of 
    //agents to be bulk processed by gpu
    bool ret = scheduler->processNextAgentEvents(&agent);

    if(agent != NULL){
        //add agent to list of agents that need equations run
        oclAgents.push_back(agent);
    }
    return ret;
}

void oclSimulation::start(){
    // This check below should be removed -- If no agents registered
    // we need to leave start and end sim
    if (allAgents.empty()) return;
    // Next initialize all the agents
    initAgents();
    //Set up OpenCL device if needed
    if(oclAvailable) initOCL();

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
//        std::cout << "Change GVT: " << gvtManager->getGVT() << std::endl;
        if (--gvtTimer == 0 ) {
            gvtTimer = gvtDelayRate;
            // Initate another round of GVT calculations if needed.
            gvtManager->startGVTestimation();
        }
        // Process a block of events received via the network, while
        // performing exponential backoff as necessary.
        checkProcessMpiMsgs();
        // Process the nextf event from the list of events managed by
        // the scheduler.
        if (!processNextEvent()) {
            // We did not have any events to process. So check MPI
            // more frequently.
            mpiMsgCheckCounter = 1;
        }
    }
    // Wait for all the parallel processes to complete the main
    // simulation loop.
    MPI_BARRIER();
}

void oclSimulation::initOCL(){
    Platform default_platform = getPlatform();
    Device default_device     = getDevice(default_platform, 1, false);
    Context context(default_device); 
    Program::Sources sources;

    std::string kernel_code = getKernel(ssa);
    sources.push_back({kernel_code.c_str(), kernel_code.length()});

    Program program(context, sources);
    if (program.build({default_device}) != CL_SUCCESS) {
        cout << "Error building: " << program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(default_device) << std::endl;
        exit(1);
    }

    CommandQueue queue(context, default_device);
//    buffer_A(context, CL_MEM_READ_WRITE, sizeof(float) * this->n);
    
//    if(ssa){
//        float randomValues[1024];
//        srand((unsigned)time(0));
//        for(int i = 0; i < 1024; i++){
//            randomValues[i] = (float)rand()/RAND_MAX;
//        }
//        queue.enqueueWriteBuffer(buffer_A, CL_TRUE, 0, sizeof(float)*1024, randomValues);
//    }
    run = Kernel(program, "run");
}

Platform oclSimulation::getPlatform() {
    std::vector<Platform> all_platforms;
    Platform::get(&all_platforms);

    if (all_platforms.size()==0) {
        cout << "No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    // for(unsigned int i = 0; i < all_platforms.size(); i++)
        // cout << "Using platform: "<< all_platforms[i].getInfo<CL_PLATFORM_NAME>() <<"\n";
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
    // for (unsigned int j=0; j<all_devices.size(); j++){
    //     cout << "Device " << j << " : " << all_devices[j].getInfo<CL_DEVICE_TYPE>() << endl;
    //     cout << "Device " << j << " : " << all_devices[j].getInfo<CL_DEVICE_EXECUTION_CAPABILITIES>() << endl;
    //     cout << "Device " << j << " : " << all_devices[j].getInfo< CL_DEVICE_VERSION >() << endl;
    //     cout << "Device " << j << " : " << all_devices[j].getInfo< CL_DEVICE_NAME >() << endl;
    // }
    return all_devices[i];
}

std::string oclSimulation::getKernel(bool ssa){
    if(ssa){
        std::string ssa_kernel_code = "void kernel run(global float* cv, global float* random, global int* rnd, global int* step) {\n"
        "    int compartments = 4;\n"
        "    float stp = 0.1;\n"
        "    float MU = 0.011;\n"
        "    float A = 0.5;\n"
        "    float V = 0.3333;\n"
        "    float R0 = 3;\n"
        "    float B  = (R0 * ((MU+A)*(MU+V)) / A);\n"
        "    float N = cv[0] + cv[1] + cv[2] + cv[3];\n"
        "    const float rates[] = { MU*N, MU*cv[0], (B * cv[2] * cv[0] / N), (A * cv[1]), ((V + MU) * cv[2]), (MU * cv[3]) };\n"
        "    const float EventChanges[6][4] = {\n"
        "               {+1, 0,  0,  0},\n"  
        "               {-1, 0,  0,  0},\n" 
        "               {-1, 1,  0,  0},\n" 
        "               {0, -1,  1,  0},\n" 
        "               {0,  0, -1, +1},\n"
        "               {0,  0,  0, -1}\n"
        "               };\n"
        "    for(int i = 0; (i < 6); i++) {\n"
        "        for(int j = 0; j < compartments; j++){\n"
        "           float scale = rates[i]*random[rnd[0]]*stp;\n"
        "           cv[j] = cv[j] + (EventChanges[i][j] * scale);\n"
        "        }\n"
        "    }\n"
        "}\n";
        return ssa_kernel_code;
    }else{
        std::string ode_kernel_code = "void kernel run(global float* xl) {\n"
        "const float h = (float)0.1;\n"
        "float MU = (float)0.011;\n"
        "float A = (float)0.5;\n"
        "float V = (float)0.3333;\n"
        "float R0 = (float)3;\n"
        "float B  = (float)(R0 * ((MU+A)*(MU+V)) / A);\n"
        "float N = (float)10000;\n"
        "int id = get_global_id(0)*4;\n"
        "  \n"
        "        // Use runge-kutta 4th order method here.\n"
        "        float k1[4], k2[4], k3[4], k4[4], xlt[4];\n"
        "        // Compute k1\n"

        "    k1[0] = (MU * N) - (MU * xl[id+0]) - (B * xl[id+2] * xl[id+0] / N);\n"
        "    k1[1] = (B * xl[id+2] * xl[id+0] / N) - (A * xl[id+1]);\n"
        "    k1[2] = (A * xl[id+1]) - ((V + MU) * xl[id+2]);\n"
        "    k1[3] = (V * xl[id+2]) - (MU * xl[id+3]);\n"

        "        // Compute k2 values using k1.\n"
        "        xlt[0] = xl[id+0] + k1[0] * h / 2;\n"
        "        xlt[1] = xl[id+1] + k1[1] * h / 2;\n"
        "        xlt[2] = xl[id+2] + k1[2] * h / 2;\n"
        "        xlt[3] = xl[id+3] + k1[3] * h / 2;\n"

        "    k2[0] = (MU * N) - (MU * xlt[0]) - (B * xlt[2] * xlt[0] / N);\n"
        "    k2[1] = (B * xlt[2] * xlt[0] / N) - (A * xlt[1]);\n"
        "    k2[2] = (A * xlt[1]) - ((V + MU) * xlt[2]);\n"
        "    k2[3] = (V * xlt[2]) - (MU * xlt[3]);\n"

        "        // Compute k3 values using k2.\n"
        "        xlt[0] = xl[id+0] + k2[0] * h / 2;\n"
        "        xlt[1] = xl[id+1] + k2[1] * h / 2;\n"
        "        xlt[2] = xl[id+2] + k2[2] * h / 2;\n"
        "        xlt[3] = xl[id+3] + k2[3] * h / 2;\n"

        "    k3[0] = (MU * N) - (MU * xlt[0]) - (B * xlt[2] * xlt[0] / N);\n"
        "    k3[1] = (B * xlt[2] * xlt[0] / N) - (A * xlt[1]);\n"
        "    k3[2] = (A * xlt[1]) - ((V + MU) * xlt[2]);\n"
        "    k3[3] = (V * xlt[2]) - (MU * xlt[3]);\n"

        "        // Compute k4 values using k3.\n"
        "        xlt[0] = xl[id+0] + k3[0] * h;\n"
        "        xlt[1] = xl[id+1] + k3[1] * h;\n"
        "        xlt[2] = xl[id+2] + k3[2] * h;\n"
        "        xlt[3] = xl[id+3] + k3[3] * h;\n"

        "    k4[0] = (MU * N) - (MU * xlt[0]) - (B * xlt[2] * xlt[0] / N);\n"
        "    k4[1] = (B * xlt[2] * xlt[0] / N) - (A * xlt[1]);\n"
        "    k4[2] = (A * xlt[1]) - ((V + MU) * xlt[2]);\n"
        "    k4[3] = (V * xlt[2]) - (MU * xlt[3]);\n"

        "\n"
        "        // Compute the new set of values.\n"
        "        xl[id+0] = xl[id+0] + (k1[0] + 2*k2[0] + 2*k3[0] + k4[0]) * h / 6;\n"
        "        xl[id+1] = xl[id+1] + (k1[1] + 2*k2[1] + 2*k3[1] + k4[1]) * h / 6;\n"
        "        xl[id+2] = xl[id+2] + (k1[2] + 2*k2[2] + 2*k3[2] + k4[2]) * h / 6;\n"
        "        xl[id+3] = xl[id+3] + (k1[3] + 2*k2[3] + 2*k3[3] + k4[3]) * h / 6;\n"
        "    // }  \n"
        "}\n";
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
    int thrStartAgent    = agentStartID;  // First agent on current thread.
    std::cout << "Start Making Agents:" << std::endl;
    for (int i = agentStartID; (i < agentEndID); i++) {
        oclState* state = new oclState(200, 3);
        OCLAgent* agent = new OCLAgent(i, state, oclAvailable);
        kernel->registerAgent(agent, currThread);

        // Handle assigning agents to different threads
        if ((++currThrNumAgents >= agentsPerThread) &&
            (currThread < threadsPerNode - 1)) {
            currThread++;          // assign agents to next thread.
            currThrNumAgents = 0;  // reset number of agents on this thread.
            thrStartAgent = i + 1; // Set starting agent for new thread
        }
//        std::cout << "Agent " << i << " created" << std::endl;

    }
    std::cout << "Registered agents from "
              << agentStartID    << " to "
              << agentEndID      << " agents.\n";
}

oclSimulation*
oclSimulation::initializeSimulation(int& argc, char* argv[], bool initMPI)
    throw (std::exception) {
    // First use a temporary argument parser to determine type of
    // simulation kernel to instantiate.
    std::string simName = "default";
    ArgParser::ArgRecord arg_list[] = {
        { "--simulator", "The type of simulator/kernel to use; one of: " \
          "default, mpi-mt, mpi-mt-shm", 
          &simName, ArgParser::STRING },
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

   return kernel;
}

void
oclSimulation::simulate() {
    // Convenient local reference to simulation kernel
    // Setup start and end time of the simulation
    std::cout << "****Simulate****" << std::endl;
    kernel->setStartTime(0);
    kernel->setStopTime(20);
    
    // Finish all the setup prior to starting simulation.
    kernel->preStartInit();
    
    //Create Agents for simulation after start and stop times are set
    createAgents();
    std::cout << "Agents Created" << std::endl;
    // Finally start the simulation here!!
    kernel->start();
    // Now we finalize the simulation to make sure it cleans up.
    finalizeSimulation();
}

bool
oclSimulation::registerAgent(muse::OCLAgent* agent, const int threadRank)  {
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
    kernel->finalize(stopMPI);
    // If the simulator is to be deleted, do that now.
    if (delSim) {
        delete kernel;  // get rid of singleton instance.
        kernel = NULL;
    }
}

void
oclSimulation::finalize(bool stopMPI, bool delCommMgr) {
    // Inform the scheduler that the simulation is complete
    scheduler->stop();
    // Finalize all the agents on this MPI process while accumulating stats
    for (AgentContainer::iterator it = allAgents.begin();
	 it != allAgents.end(); it++) {
	Agent* const agent = *it;
        agent->finalize();
        agent->garbageCollect(TIME_INFINITY);
        agent->cleanStateQueue();
        agent->cleanInputQueue();
        // Don't clean output queue yet as we need stats from it.
    }
    // Report aggregate statistics from this kernel
//    reportStatistics();

    // Clean up all the agents
    for (AgentContainer::iterator it = allAgents.begin();
	 it != allAgents.end(); it++) {
        Agent* const agent = *it;
        agent->cleanOutputQueue();
        // Remove agent from scheduler
        scheduler->removeAgentFromScheduler(agent);
        // Bye byte agent!
        delete agent;
    }

    // Now delete GVT manager as we no longer need it.
    commManager->setGVTManager(NULL);
    delete gvtManager;
    gvtManager = NULL;
    
    // Finalize the communicator 
    commManager->finalize(stopMPI);
    // Delete it if requested
    if (delCommMgr) {
        delete commManager;
    }
    commManager = NULL;

    // Invalidate the  kernel ID
    myID = -1u;

    DEBUG({
            if (logFile != NULL) {
                // Un-hijack cout
                std::cout.rdbuf(oldstream);
                // Get rid of the log file.
                delete logFile;
                logFile = NULL;
            }
    });
    // Get rid of scheduler as we no longer need it
    delete scheduler;
    scheduler = NULL;
    // Clear out the list of agents in this simulation
    allAgents.clear();
    // Finally clear out any pending events in the event recyler.
    EventRecycler::deleteRecycledEvents();
    // Clear out any pending states in the event recycler
    StateRecycler::deleteRecycledStates();
}

END_NAMESPACE(muse);

int 
main(int argc, char** argv) {
    muse::oclSimulation* sim = new muse::oclSimulation(); 
    std::cout << "start" << std::endl;
    sim->initializeSimulation(argc, argv, true);
    std::cout << "Initialized Simulation" << std::endl;
    sim->simulate();
    return 0;
}


#endif