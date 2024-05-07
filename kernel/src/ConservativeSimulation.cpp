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
// Authors: Jingbin Yu             yuj53@miamioh.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "ConservativeSimulation.h"
#include "ArgParser.h"
#include "Communicator.h"
#include "SimpleGVTManager.h"
#include "Scheduler.h"
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>

muse::ConservativeSimulation::ConservativeSimulation() {
    mustSaveState = false;
}

muse::ConservativeSimulation::~ConservativeSimulation() {

}

void muse::ConservativeSimulation::initialize(int& argc, char* argv[], bool initMPI) {
    commManager = new Communicator();
    myID = commManager->initialize(argc, argv, initMPI);
    unsigned int numThreads;
    commManager->getProcessInfo(myID, numberOfProcesses, numThreads);
    // Parse arguments from command line
    parseCommandLineArgs(argc, argv);

    // Call base initialize
    muse::Simulation::initialize(argc, argv, initMPI);
}

void muse::ConservativeSimulation::parseCommandLineArgs(int& argc, char* argv[]) {
    // First, parse base Simulation args and get rid of them
    muse::Simulation::parseCommandLineArgs(argc, argv);
    
    double cmdLookahead = -2;
    // The initial value being negative will force
    // it to tell the user that the lookAhead will
    // be 1 if they do not specify a number.
    // Using -2 will ensure that it is unlikely to
    // incur any precison issue with double data type

    ArgParser::ArgRecord arg_list[] = {
        {"--cmbLookahead", "The constant lookahead value for Conservative Simulation",
            &cmdLookahead, ArgParser::DOUBLE},
        {"", "", NULL, ArgParser::INVALID}
    };

    ArgParser argp(arg_list);
    argp.parseArguments(argc, argv, false);

    if (cmdLookahead <= 0) {
        cmdLookahead = 1;
        std::cerr << "Lookahead value for Conservative Simulation must be positive, using 1" << std::endl;
    } else {
        std::cout << "Using Lookahead value " << cmdLookahead << std::endl;
    }

    lookAhead = cmdLookahead;
}

void muse::ConservativeSimulation::preStartInit() {
    ASSERT(commManager != nullptr);
    gvtManager = new SimpleGVTManager(this);
    gvtManager->initialize(startTime, commManager);

    commManager->setGVTManager(gvtManager);
    commManager->registerAgents(allAgents);

    scheduler->start(startTime);
}

int muse::ConservativeSimulation::processMpiMsgs() {
    // If we have only one process then there is nothing to be done
    if (numberOfProcesses < 2) {
        return 0;  // no other process to communicate with.
    }
    // Track number of times the processMpiMsgs method is called.
    processMpiMsgCalls++;
    // An optimization trick is tseo try to get as many events from
    // the wire as we can. A good magic number is 100.  However
    // this number could be dynamically adapted depending on
    // behavior of the simulation.
    int numMsgs;
    for (numMsgs = 0; (numMsgs < maxMpiMsgThresh); numMsgs++) {
        Event* incoming_event = commManager->receiveEvent();
        if (incoming_event != NULL) {
            ASSERT(incoming_event->getReferenceCount() == 1);

            scheduleEvent(incoming_event);
            // Decrease the reference because if it was rejected,
            // the event will be properly deleted. However, if it
            // is in the eventPQ, it will be unharmed (reference
            // count will actually be fixed to correct for lack of
            // local output queue)
            ASSERT(EventRecycler::getReferenceCount(incoming_event) < 3);
            EventRecycler::decreaseReference(incoming_event);
            // Time topEventTime = scheduler->getNextEventTime();
            // LGVT = topEventTime;
            if ((LGVT = scheduler->getNextEventTime()) < getGVT()) {
                std::cout << "LGVT: " << LGVT  << " is below GVT: " << getGVT()
                          << " which is serious error. "
                          << "Scheduled agents:\n";
                scheduler->agentPQ->prettyPrint(std::cout);
                std::cerr << "Rank " << myID << " Aborting.\n";
                std::cerr << std::flush;
                DEBUG(logFile->close());
                ASSERT ( false );
            }
        } else {
            break;  // No message current available.
        }
    }

    // Track the batch size if numMsgs is greater than zero.
    if (numMsgs > 0) {
        mpiMsgBatchSize += numMsgs;
    }

    return numMsgs;
}

void muse::ConservativeSimulation::start() {
    // If this process has no agent, we do nothing
    if (allAgents.empty()) return;

    preStartInit();
    initAgents();

    LGVT = startTime;

    while (true) {
        processMpiMsgs();

        gvtManager->forceUpdateGVT();

        if (getGVT() >= getStopTime()) break;

        if (doDumpStats) {
            dumpStats();
            doDumpStats = false;
        }

        // Then, we start to process events
        processNextEvent();
    }
    
    MPI_BARRIER();
}

bool muse::ConservativeSimulation::processNextEvent() {
    // First, we get the most recent future event to process
    // We have to use the time of this event to decide 

    Time nextEventTime = scheduler->getNextEventTime();

    LGVT = nextEventTime;

    // Return immediately if there is no event to be processed
    // This check is done first so that if we do not have any
    // event to schedule, we do not have to try to update GVT,
    // which is time-consuming to do
    if (nextEventTime == TIME_INFINITY) {
        return false;
    }

    // while (nextEventTime >= getGVT() + lookAhead) {
    //     // We cannot process the current event, so
    //     // we just check if there are other events
    //     // coming, and then update GVT. Do this
    //     // until we can process an event
    //     processMpiMsgs();
    //     nextEventTime = scheduler->getNextEventTime();
    //     LGVT = nextEventTime;
    //     gvtManager->forceUpdateGVT();
    // }


    if (nextEventTime >= getGVT() + lookAhead) {
        return false;
    }

    Time timeOfEventToProcess = nextEventTime;

    // If the control reaches here, it means
    // it is safe to process the top event

    // We call scheduler->processNextAgentEvents()
    // to process all events at timeOfEventToProcess of
    // the top agent
    // Our goal is to process all events in
    // all agents, so we do a loop
    do {
        // This will process all events with recieve time == top event time
        // from the top agent
        AgentID idOfAgentProcessed = scheduler->processNextAgentEvents();

        ASSERT(idOfAgentProcessed != InvalidAgentID);

        // This should NOT happen
        if (idOfAgentProcessed == InvalidAgentID) {
            std::cerr << "Simulation #" << myID 
            << ": Error processing event" << std::endl;
            return false;
        }

        // This will help us get the top event in the queue.
        // This top event will be from a different agent than
        // the one whose events we just processed
        nextEventTime = scheduler->getNextEventTime();

        // If the top event has receive time identical to
        // the time we need, the next loop should process
        // all the events at this time from this event's agent
    } while (nextEventTime <= timeOfEventToProcess);

    return true;
}