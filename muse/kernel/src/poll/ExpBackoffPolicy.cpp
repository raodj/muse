
#include "poll/ExpBackoffPolicy.h"

ExpBackoffPolicy::ExpBackoffPolicy() {
}

ExpBackoffPolicy::~ExpBackoffPolicy() {
}

bool ExpBackoffPolicy::shouldPoll() {
    // std::cout << "exp BO : shouldPoll" << std::endl;
    return (--mpiMsgCheckCounter == 0);
}

void ExpBackoffPolicy::updatePolicy(int numEvents) {
    // std::cout << "exp BO : updateState - " << numEvents << std::endl;
    if (numEvents == 0) {
        mpiMsgCheckThresh = std::min(10240, mpiMsgCheckThresh * 2);
    } else {
        mpiMsgCheckThresh = std::min(1, mpiMsgCheckThresh / 2);
    }
    mpiMsgCheckCounter = mpiMsgCheckThresh;
}

void ExpBackoffPolicy::forcePoll() {
    // std::cout << "exp BO : forcePoll" << std::endl;
    mpiMsgCheckCounter = 1;
}
