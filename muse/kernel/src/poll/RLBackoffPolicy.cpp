
#include "poll/RLBackoffPolicy.h"

RLBackoffPolicy::RLBackoffPolicy() {
}

RLBackoffPolicy::~RLBackoffPolicy() {
}

bool RLBackoffPolicy::shouldPoll() {
    return true;
}

void RLBackoffPolicy::updatePolicy(int numEvents) {
}

void RLBackoffPolicy::forcePoll() {
}
