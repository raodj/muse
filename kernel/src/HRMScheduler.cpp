#ifndef MUSE_HRM_SCHEDULER_CPP
#define MUSE_HRM_SCHEDULER_CPP

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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "HRMScheduler.h"
#include "ResChannel.h"
#include <sys/time.h>
#include <stdexcept>
#include <iterator>

using namespace muse;

static int myRank = -1;

HRMScheduler::HRMScheduler() {
    // Initialize local instance variables
    prevEpochVTime = 0;
    epochClock     = 0;
    // Set default threshold values
    epochClockThreshold = 30; // in milliseconds
    epochVTimeThreshold = 10; // virtual time (no unit)
}

HRMScheduler::~HRMScheduler() {
    // Nothing else to be done in the destructor for now.
}

void
HRMScheduler::initialize(int rank, int numProcesses, int& argc, char *argv[]) {
    ArgParser::StringList hrmDomainList;
    std::string hrmDomainListFile;
    int minEpochCount = 3;
    myRank = rank;
    // Make the arg_record
    ArgParser::ArgRecord arg_list[] = {
        {"--epoch-clock-thresh",
         "Minimum duration of execution time required for a new epoch",
         &epochClockThreshold, ArgParser::LONG},
        {"--epoch-vtime-thresh",
         "Minimum advancement in virtual time required for a new epoch",
         &epochVTimeThreshold, ArgParser::DOUBLE},
        {"--min-epoch-count", "Minimum number of epoch needed to report speed",
         &minEpochCount, ArgParser::INTEGER},
        { "--hrm-controller-list",
          "List of space separated HRM controllers (hostname:port) for each rank",
          &hrmDomainList, ArgParser::STRING_LIST},
        { "--hrm-controller-file",
          "Text file with list of HRM controllers for each rank",
          &hrmDomainListFile, ArgParser::STRING},
        {"", "", NULL, ArgParser::INVALID}
    };
    // Use the argument parser to parse command-line arguments and
    // update local variables
    ArgParser ap(arg_list);
    ap.parseArguments(argc, argv, false);
    // Select the domain controller to talk to from command-line arguments.
    const std::string ctrlInfo =
        getDomainControllerInfo(hrmDomainList, hrmDomainListFile, rank);
    // Print HRM scheduler configuration information to the user.
    std::cout << "HRMScheduler configuration on rank " << rank
              << " . Num processes is "                << numProcesses
              << "\n  Epoch clock threshold        : " << epochClockThreshold
              << "\n  Epoch virtual time threshold : " << epochVTimeThreshold
              << "\n  Epoch count for speed reports: " << minEpochCount
              << "\n  Domain controller information: " << ctrlInfo
              << std::endl;
    // Now, have our resource channel handler connect to the domain.
    resChannel.setMinEpochCount(minEpochCount);
    resChannel.connect(rank, ctrlInfo);
}

std::string
HRMScheduler::getDomainControllerInfo(ArgParser::StringList& cmdLineList,
                                      const std::string& filePath, int rank) const {
    if (cmdLineList.empty() && filePath.empty()) {
        std::cerr << "HRMScheduler: Specify list of HRM domain controllers to "
                  << "use via --hrm-controller-list or --hrm-controller-file "
                  << "command-line arguments. Aborting.\n";
        throw std::runtime_error("Insufficient command-line arguments.");
    } else if (cmdLineList.empty()) {
        // Load list of domain controllers from the given text file
        std::ifstream listFile(filePath.c_str());
        if (!listFile.good()) {
            std::cerr << "HRMScheduler: Unable to open domain controller "
                      << "information from file: " << filePath << std::endl;
            throw std::runtime_error("Invalid domain controller file path");
        }
        // Use iterators to load information into cmdLineList
        std::istream_iterator<std::string> infoReader(listFile), eof;
        cmdLineList.insert(cmdLineList.end(), infoReader, eof);
    }
    // Now we should have at least 'rank' number of entries in cmdLineList
    if (cmdLineList.size() <= (size_t) rank) {
        std::cerr << "HRMScheduler: Did not find domain controller information "
                  << "for process with rank " << rank << std::endl;
        throw std::runtime_error("Insufficient domain controller information.");
    }
    // So far so good...
    return cmdLineList[rank];
}

void
HRMScheduler::start(const Time& startTime) {
    UNUSED_PARAM(startTime);
    resChannel.informStart();
}

AgentID
HRMScheduler::processNextAgentEvents() {
    // Save the time of the next set of events to be processed by the
    // agent to detect occurrence of next Epoch.
    const Time nextEvtTime = getNextEventTime();
    // Track the real time taken to advance through Epochs
    if ((epochClock == 0) || (prevEpochVTime == 0)) {
        // This case happens during initialization or after rollback!
        epochClock    = currentTimeMillis();
        prevEpochVTime = nextEvtTime;
    }
    // Process next set of events (if any).
    AgentID  retVal = InvalidAgentID;
    if ((nextEvtTime != INFINITY) && (retVal = Scheduler::processNextAgentEvents())) {
        // Check to see if an Epoch of sufficient duration has occurred.
        unsigned long currTime = 0;
        if (((nextEvtTime - prevEpochVTime) > epochVTimeThreshold) &&
            (((currTime = currentTimeMillis()) - epochClock) > epochClockThreshold)) {
            // New Epoch has been detected.  Append it to the epoch list.
            Epoch epoch(prevEpochVTime, nextEvtTime,
                        nextEvtTime - prevEpochVTime, currTime - epochClock);
            std::cout << "Rank: " << myRank << ": Created new epoch: "
                      << epoch.toString() << std::endl;
            resChannel.add(epoch, currTime);
            // Update epoch tracking variables
            prevEpochVTime = nextEvtTime;
            epochClock     = currTime;
        }
    }
    return retVal;
}

void
HRMScheduler::garbageCollect(const Time& gvt) {
    resChannel.garbageCollect(gvt);
}

bool
HRMScheduler::checkAndHandleRollback(const Event* event, Agent* agent) {
    // Let base class do the necessary work if a rollback actually occurred
    const Time evtTime = event->getReceiveTime();
    const bool retVal  = Scheduler::checkAndHandleRollback(event, agent);
    if (retVal) {
        // A rollback really happened due to this event! Clear out
        // Epochs
        resChannel.rollback(evtTime);
        std::cout << "Rank " << myRank << ": Epochs after rollback to "
                  << evtTime << " = " << resChannel.getEpochCount() << std::endl;
        // Reset Epoch tracking counters so that new epochs are tracked
        epochClock     = 0;
        prevEpochVTime = 0;
    }
    return retVal;
}

unsigned long
HRMScheduler::currentTimeMillis() const {
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    return ((currTime.tv_sec * 1000UL) + (currTime.tv_usec / 1000UL));
}

#endif
