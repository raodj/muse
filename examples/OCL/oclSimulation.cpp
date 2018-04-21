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
    if (kernel->allAgents.empty()) return;
//     Set up OpenCL device if needed
    if(oclAvailable) {
        cout << "Initialize Agents" << endl;
        // Next initialize all the agents
        initAgents();
        cout << "Initialize OCL" << endl;
        // initialize opencl
        initOCL((oclAgent*)kernel->allAgents[0]);
        // run ocl sim loop
        
        // ---comment function call for reference passing solution --- CHECK
        oclRun();

    }else{
        cout << "Run base sim loop" << endl;
        Simulation::start();//run();
        
    }
    // Wait for all the parallel processes to complete the main
    // simulation loop.
    MPI_BARRIER();
}

cl::Platform oclSimulation::getPlatform() {
    std::vector<cl::Platform> all_platforms;
    cl::Platform::get(&all_platforms);

    if (all_platforms.size()==0) {
        cout << "No platforms found. Check OpenCL installation!\n";
        exit(1);
    }
    return all_platforms[0];
}

cl::Device oclSimulation::getDevice(cl::Platform platform, int i, bool display) {
    std::vector<cl::Device> all_devices;
    platform.getDevices(CL_DEVICE_TYPE_ALL, &all_devices);
    if(all_devices.size()==0){
        cout << "No devices found. Check OpenCL installation!\n";
        exit(1);
    }

    if (display) {
        for (unsigned int j=0; j<all_devices.size(); j++)
            printf("Device %d: %s\n", j, all_devices[j].getInfo<CL_DEVICE_NAME>().c_str());
    }
    return all_devices[i];
}

void
oclSimulation::createAgents() {
    const int max_nodes      = 1; // getNumberOfProcesses();
    const int threadsPerNode = 1; // getNumberOfThreads();
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
        oclAgent* agent = new synthAgent(i, state, oclAvailable, step, compartments, ode);
//        oclAgent* agent = new oclAgent(i, state, kernel->oclAvailable, kernel->step, compartments, kernel->ode);
        kernel->registerAgent(agent, currThread);
        agent->initialize();


        // Handle assigning agents to different threads
        if ((++currThrNumAgents >= agentsPerThread) &&
            (currThread < threadsPerNode - 1)) {
//            currThread++;          // assign agents to next thread.
            currThrNumAgents = 0;  // reset number of agents on this thread.
        }

    }
//    std::cout << "Registered agents from "
//              << agentStartID    << " to "
//              << agentEndID      << " agents.\n";
}

Simulation*
oclSimulation::initializeSimulation(int& argc, char* argv[], bool initMPI)
    throw (std::exception) {
    int row = 1;
    int col = 1;
    bool ocl = true;
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
    rows = row;
    cols = col;
    oclAvailable = ocl;
    stopTime = stop;
    popSize = pop;
    expSize = exp;
    step = stp;
    compartments = comp;
    ode = !ssa;
    maxWorkGroups = maxWG;
    kernel = Simulation::initializeSimulation(argc, argv, initMPI);
    kernel->scheduler = new oclScheduler();
    commManager = new Communicator();
    return kernel;
}

void
oclSimulation::simulate() {
    
    // Convenient local reference to simulation kernel
    // Setup start and end time of the simulation
    kernel->setStartTime(0);
    kernel->setStopTime(stopTime);
    cout << "Set Start + Stop" << endl;
    // Finish all the setup prior to starting simulation.
    preStartInit();
    cout << "Create Agents" << endl;
    //Create Agents for simulation after start and stop times are set
    createAgents();
    cout << "Created Agents" << endl;
//     Finally start the simulation here!!
    oclSimulation::start();
    cout << "Finished Simulation Loop" << endl;
    // Now we finalize the simulation to make sure it cleans up.
//    finalizeSimulation();
}

void oclSimulation::initOCL(oclAgent* agent){
    
    // determine maximum number of agents to send to the GPU for processing
    int maxGlobal = std::min(CL_DEVICE_MAX_WORK_GROUP_SIZE*100, rows*cols*compartments);
    // if user specified number of agents, use that value if less than the current max
    if(maxWorkGroups != 0){
        maxGlobal = std::min(maxWorkGroups*compartments, maxGlobal);
    }
    
    // initialize opencl platform and devices 
    cl::Platform plat = getPlatform();
    cl::Device dev     = getDevice(plat, 0, false);
    cl::Context ctx(dev); 
    cl::Program::Sources src;

    std::string kernel_code = agent->getKernel();
    src.push_back({kernel_code.c_str(), kernel_code.length()});

    cl::Program prog(ctx, src);
    if (prog.build({dev}) != CL_SUCCESS) {
        cout << "Error building: " << prog.getBuildInfo<CL_PROGRAM_BUILD_LOG>(dev) << std::endl;
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
    srand ( time(NULL) );
    if(!ode){
        float randomValues[numRand];
        for(int i = 0; i < numRand; i++){
            randomValues[i] = static_cast <real> (rand()) / static_cast <real> (RAND_MAX);
        }
        q.enqueueWriteBuffer(buffer_rnd, CL_TRUE, 0, sizeof(real)*(numRand), randomValues);
    }
    cout << "Initialized OCL" << endl;

    // CHECK -- comment class variable assignment, uncomment function call
    queue = &q;
    run = &r;
    buffer_B = &B;
    buffer_random = &buffer_rnd;
    buffer_compartments = &buffer_comp;
//    oclRun(&q, &r, &B, &buffer_rnd);
}
// CHECK -- switch function definition
//void oclSimulation::oclRun(cl::CommandQueue* queue, cl::Kernel* run, cl::Buffer* buffer_B, cl::Buffer* buffer_random){
void oclSimulation::oclRun(){
        // determine maximum number of agents to send to the GPU for processing
    int maxGlobal = std::min(CL_DEVICE_MAX_WORK_GROUP_SIZE*100, rows*cols*compartments);
    // if user specified number of agents, use that value if less than the current max
    if(maxWorkGroups != 0){
        maxGlobal = std::min(maxWorkGroups*compartments, maxGlobal);
    }
    // initialize array to hold agent values to be passed onto GPU
    real v[maxGlobal];
    values = v;
    
    // Start the core simulation loop.
    LGVT         = startTime;
    lastLGVT = startTime;
    int gvtTimer = 2;
    // The main simulation loop
//    cout << "start sim loop: " << gvtManager->getGVT() << ", " << kernel->endTime << endl;
    while (gvtManager->getGVT() < kernel->endTime) { 
//            cout << "enter sim loop: " << LGVT << ", " << gvtManager->getGVT() << endl;

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
            LGVT = kernel->scheduler->getNextEventTime();
            if((LGVT > lastLGVT && oclAvailable && !oclAgents.empty()) || oclAgents.size() >= maxGlobal/compartments){
                lastLGVT = LGVT;
                // CHECK -- switch function calls
//                processWithOCL(queue,run, buffer_B, buffer_random, maxGlobal);
                processWithOCL(maxGlobal);
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
            scheduler = (oclScheduler*)kernel->scheduler;
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
//    queue->finish();
}

void
oclSimulation::initialize(int& argc, char* argv[], bool initMPI)
    throw (std::exception) {
    Simulation::initialize(argc, argv, initMPI);
    commManager = new Communicator();
    myID = commManager->initialize(argc, argv, initMPI);
    unsigned int numThreads;  // dummy. not really used.
    commManager->getProcessInfo(myID, numberOfProcesses, numThreads);
}

void
oclSimulation::finalize(bool stopMPI, bool delCommMgr) {
    // Let base class do the necessary work (for now)
    Simulation::finalize(stopMPI, delCommMgr);
}

void
oclSimulation::preStartInit() {
    gvtManager = new GVTManager(this);
    gvtManager->initialize(startTime, commManager);
    // Set gvt manager with the communicator.
    commManager->setGVTManager(gvtManager);
    // Inform scheduler(s) that simulation is starting.
    kernel->scheduler->start(startTime);
    // Next, we setup/finalize the AgentMap for all kernels
    commManager->registerAgents(allAgents);
}

void
oclSimulation::garbageCollect() {
    const Time gvt = getGVT();
    // First let the scheduler know it can garbage collect.
    kernel->scheduler->garbageCollect(gvt);
    for (AgentContainer::iterator it = allAgents.begin();
         (it != allAgents.end()); it++) {  
        (*it)->garbageCollect(gvt);
    }
}

// CHECK -- switch function definition
void 
//oclSimulation::processWithOCL(cl::CommandQueue* queue, cl::Kernel* run, cl::Buffer* buffer_B, cl::Buffer* buffer_random, int maxGlobal){
oclSimulation::processWithOCL(int maxGlobal){

    std::cout<<"---Doing Bulk Processing at "<< lastLGVT  << ", " << oclAgents.size()<< " ---" << std::endl;        

    // parse data from agents
    int count = oclAgents.size();
    int c = 0;
    for(oclAgent* agent : oclAgents){
        for(int i = 0; i < compartments; i++){
            values[i+c] = agent->myState->values[i];      
        }
        c+=compartments;
    }


    // copy data onto GPU
    cl_int wstatus = queue->enqueueWriteBuffer(*buffer_B, CL_TRUE, 0, sizeof(real)*(maxGlobal), values);
    std::cout << "Write Status: " << wstatus << std::endl;
    // Set args for kernel code
    cl_int astatus = run->setArg(0, *buffer_B);
    if(!ode){
        run->setArg(1, buffer_random);
    }

//                std::cout << "Set Arg Status: " << astatus << std::endl;
    // run kernel code on agent data
    // the last input into enqueueNDRangeKernel can specify the local work size, but leaving it null allows the opencl to determine the local work size
    queue->flush();
    cl_int status = queue->enqueueNDRangeKernel(*run, cl::NullRange, cl::NDRange(count), cl::NullRange);
    queue->flush();
    std::cout << "Status: " << status << std::endl;
    // read data back to values array
    status = queue->enqueueReadBuffer(*buffer_B, CL_TRUE, 0, sizeof(real)*(maxGlobal), values);
    std::cout << "Read Status: " << status << std::endl;
    queue->flush();

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

END_NAMESPACE(muse);

int 
main(int argc, char** argv) {
    muse::oclSimulation* sim = new muse::oclSimulation();
    sim->initializeSimulation(argc, argv, true);
    std::cout << "Initialized Simulation" << std::endl;
    sim->simulate();
    return 0;
}

#endif