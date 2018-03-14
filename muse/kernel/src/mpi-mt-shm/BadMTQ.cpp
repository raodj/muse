#ifndef BAD_MT_Q_CPP
#define BAD_MT_Q_CPP

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
// Authors: Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include <thread>

#include "mpi-mt-shm/BadMTQ.h"

BEGIN_NAMESPACE(muse)

std::recursive_mutex BadMTQ::naiveMutex;

BadMTQ::BadMTQ() {
	std::cout << "Made a Bad MT Q" << std::endl;
}

BadMTQ::~BadMTQ() {}

void* BadMTQ::addAgent(Agent* agent) {
	// lock it
//	std::lock_guard<std::recursive_mutex> guard(naiveMutex);
	// do it
        agentMap[agent->getAgentID()] = agent;
	return ThreeTierHeapEventQueue::addAgent(agent);
}

void BadMTQ::dequeueNextAgentEvents(muse::EventContainer& events) {
    ASSERT(events.empty());
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    if (!empty()) {
        if (!agentMap[front()->getReceiverAgentID()]->agentLock.try_lock()) {
            return; // if we can't get the agent lock, another thread is already
            // working with the agent, just skip and come back later
        }
    }
    ThreeTierHeapEventQueue::dequeueNextAgentEvents(events);
}

void BadMTQ::removeAgent(Agent* agent) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    ThreeTierHeapEventQueue::removeAgent(agent);
}

bool BadMTQ::empty() {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::empty();
}

muse::Event* BadMTQ::front() {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::front();
}

void BadMTQ::enqueue(muse::Agent* agent, muse::Event* event) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    ThreeTierHeapEventQueue::enqueue(agent, event);
}

void BadMTQ::enqueue(muse::Agent* agent, muse::EventContainer& events) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    ThreeTierHeapEventQueue::enqueue(agent, events);
}

int BadMTQ::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
        const muse::Time sentTime) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::eraseAfter(dest, sender, sentTime);
}

void BadMTQ::enqueueEvent(muse::Agent* agent, muse::Event* event) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);

    // do it
    ThreeTierHeapEventQueue::enqueueEvent(agent, event);

    // std::thread::id tid = std::this_thread::get_id();
    // std::cout << "Added event for   " << agent->getAgentID() << " time " << event->getReceiveTime() << " - " << tid << std::endl;
}

void BadMTQ::pop_front(muse::Agent* agent) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    ThreeTierHeapEventQueue::pop_front(agent);
}

muse::Agent* BadMTQ::top() {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::top();
}

muse::Time BadMTQ::getTopTime(const muse::Agent* const agent) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::getTopTime(agent);
}

bool BadMTQ::compare(const Agent *lhs, const Agent * rhs) const {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::compare(lhs, rhs);
}

void BadMTQ::getNextEvents(Agent* agent, EventContainer& container) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    ThreeTierHeapEventQueue::getNextEvents(agent, container);
}

size_t BadMTQ::updateHeap(muse::Agent* agent) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::updateHeap(agent);
}

size_t BadMTQ::fixHeap(size_t currPos) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::fixHeap(currPos);
}

HOETier2Entry* BadMTQ::makeTier2Entry(muse::Event* event) {
    // lock it
    std::lock_guard<std::recursive_mutex> guard(naiveMutex);
    // do it
    return ThreeTierHeapEventQueue::makeTier2Entry(event);
}

END_NAMESPACE(muse)

#endif
