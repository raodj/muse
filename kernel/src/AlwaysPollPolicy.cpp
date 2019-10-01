
#include "AlwaysPollPolicy.h"

AlwaysPollPolicy::AlwaysPollPolicy() {
}

AlwaysPollPolicy::~AlwaysPollPolicy() {
}

/**
 * Always poll
 * 
 * @return Always return true
 */
bool AlwaysPollPolicy::shouldPoll() {
    // std::cout << "always : shouldPoll" << std::endl;
    return true;
}

/**
 * Nothing to do for this policy
 * 
 * @param numEvents
 */
void AlwaysPollPolicy::updatePolicy(int numEvents) {
    // std::cout << "always : updateState - " << numEvents << std::endl;
}

/**
 * 
 */
void AlwaysPollPolicy::forcePoll() {
    // std::cout << "always : forcePoll" << std::endl;
}
