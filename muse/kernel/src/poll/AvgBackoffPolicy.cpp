
#include "poll/AvgBackoffPolicy.h"

AvgBackoffPolicy::AvgBackoffPolicy() {
}

AvgBackoffPolicy::~AvgBackoffPolicy() {
}

bool AvgBackoffPolicy::shouldPoll() {
    // std::cout << "avg BO : shouldPoll" << std::endl;
    return (--mpiMsgCheckCounter == 0);
}

void AvgBackoffPolicy::updatePolicy(int numEvents) {
    // std::cout << "avg BO : updateState - " << numEvents << std::endl;
    if (numEvents == 0) {
        if ((mpiMsgCheckThresh == 1) &&
            (maxMpiMsgCheckThresh.getCount() > 100)) {
            // Once we have collected sufficient samples we can
            // setup our threshold directly to that value.
            mpiMsgCheckThresh = maxMpiMsgCheckThresh.getMean();
        } else {
            mpiMsgCheckThresh = std::min(128, mpiMsgCheckThresh + 1);
        }
    } else {
        // Track the maximum value for mpiMsgCheckThresh for use
        // later on.
        maxMpiMsgCheckThresh += mpiMsgCheckThresh;
        mpiMsgCheckThresh     = 1;  // Reset back to 1.
    }
    mpiMsgCheckCounter = mpiMsgCheckThresh;
}

void AvgBackoffPolicy::forcePoll() {
    // std::cout << "avg BO : forcePoll" << std::endl;
    mpiMsgCheckCounter = 1;
}
