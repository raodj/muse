#ifndef PCS_Agent_CPP
#define PCS_Agent_CPP

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

#include "PCS_Agent.h"

PCS_Agent::PCS_Agent(muse::AgentID id, PCS_State* state, int x, int y, int n,
                   double call_interval_mean, double call_duration_mean,
                   double move_interval_mean, int lookAhead,
                   size_t granularity) :
    Agent(id, state), X(x), Y(y), N(n), lookAhead(lookAhead),
    granularity(granularity), callIntervalMean(call_interval_mean),
    callDurationMean(call_duration_mean),
    moveIntervalMean(move_interval_mean), generator(id) {
    // Setup the random seed used for generating random delays.
    seed = id;
}

NextAction
PCS_Agent::getAction(muse::Time completeCallTime, muse::Time nextCallTime,
                    muse::Time moveCallTime) const {
    NextAction action;
    // The if-checks below give lower preference to move when compared
    // to other operations to avoid time conflicts because in this
    // implementation move requires a small 0.125 increment in time
    // because in MUSE does not allow events with zero change in LVT.
    if (completeCallTime <= nextCallTime) {
        action = (completeCallTime <= moveCallTime) ? COMPLETECALL : MOVECALL;
    } else {
        action = (nextCallTime <= moveCallTime) ? NEXTCALL : MOVECALL;
    }
    return action;
}

muse::Time
PCS_Agent::minTimeStamp(muse::Time completeCallTime, muse::Time nextCallTime,
                       muse::Time moveCallTime) const {
    return std::min(completeCallTime, std::min(nextCallTime, moveCallTime));
}

PCS_Event*
PCS_Agent::getNextEvent(const uint portableID, const muse::Time callCompleteTime,
                       const muse::Time nextCallTime,
                       const muse::Time moveTime) const {
    const NextAction action = getAction(callCompleteTime, nextCallTime,
                                        moveTime);
    Method method = COMPLETE_CALL;  // Will change below.
    switch (action) {
    case COMPLETECALL: method = COMPLETE_CALL; break;
    case NEXTCALL:     method = NEXT_CALL;     break;
    case MOVECALL:     method = MOVE_OUT;      break;
    default:
        std::cerr << "Unhandled agent type encountered in PCS_Agent.cpp\n";
        ASSERT(false);
    }
    // Determine event receive time to change state of portable based
    // on minimum of 3 timestamps.
    muse::Time receive = minTimeStamp(callCompleteTime, nextCallTime, moveTime);
    PCS_Event* e = PCS_Event::create(getAgentID(), receive, moveTime,
                                    nextCallTime, callCompleteTime,
                                    method, portableID);
    return e;
}

PCS_Event*
PCS_Agent::completionCall(const PCS_Event& event) {
    // A portable has finished a call. Update necessary state
    // information and setup the portable for the next operation.
    PCS_State* const state = dynamic_cast<PCS_State*> (getState());
    // Reset call completion timestamp to infinity (as in paper)
    const muse::Time complete_call_timeStamp = TIME_INFINITY;
    // Channel held by Portable is now free. Track free channels in state.
    
    state->setIdleChannels(state->getIdleChannels() + 1);
    // Note that the next call timestamp is updated in the nextCall
    // method and does not need to be updated here -- consistent with paper!
    const muse::Time next_call_timeStamp = event.getNextCallTimeStamp();
    const muse::Time move_timeStamp      = event.getMoveTimeStamp();
    // Schedule event to change state of this portable based on
    // minimum of the 3 timestamps.
    return getNextEvent(event.getPortableID(), complete_call_timeStamp,
                        next_call_timeStamp, move_timeStamp);
}

PCS_Event*
PCS_Agent::moveIn(const PCS_Event& event) {
    PCS_State* const state = dynamic_cast<PCS_State*> (getState());
    // Track number of move-ins
    state->trackMoveIn();
    muse::Time complete_call_timeStamp = event.getCompletionTimeStamp();
    const muse::Time next_call_timeStamp = event.getNextCallTimeStamp();
    // Move-in events have a small 0.125 offset added. So the
    // type-cast below is used to get rid of that small delta. Compute
    // the next time this portable must move out of this Cell.
    std::exponential_distribution<double> move_distrib(1.0 / moveIntervalMean);
    uint moveDistrib = lookAhead + move_distrib(generator);
    const muse::Time move_timeStamp = event.getMoveTimeStamp() + moveDistrib;
    // Determine the ongoing action based on minimum of timestamps
    NextAction action = getAction(complete_call_timeStamp, next_call_timeStamp,
                                  move_timeStamp);
    if (action == COMPLETECALL) {
        // A call is in progress for this portable. Check channel
        // availability, and assign one on this Cell
        if (state->getIdleChannels() == 0) { // All channels are busy.
            state->setHandOffBlocks(state->getHandOffBlocks() + 1);
            // Call completion timestamp is reset to infinity
            complete_call_timeStamp = TIME_INFINITY;
        } else {
            // A channel is available. Allocate it.  No change in
            // timestamps as things are going on smoothly.
            state->setIdleChannels(state->getIdleChannels() - 1);
        }
    }
    // Schedule event to change state of this portable based on
    // minimum of the 3 timestamps.
    return getNextEvent(event.getPortableID(), complete_call_timeStamp,
                        next_call_timeStamp, move_timeStamp);    
}

PCS_Event*
PCS_Agent::moveOut(const PCS_Event& event) {
    PCS_State* const state = dynamic_cast<PCS_State*> (getState());
    // Track number of move-outs
    state->trackMoveOut();
    // Change state/time based on move out operation.
    muse::Time complete_call_timeStamp = event.getCompletionTimeStamp();
    muse::Time next_call_timeStamp     = event.getNextCallTimeStamp();
    muse::Time move_timeStamp          = event.getMoveTimeStamp();
    // Check if call is in progress and free channel.
    if (complete_call_timeStamp < TIME_INFINITY) {
        state->setIdleChannels(state->getIdleChannels() + 1);
    }
    // Now, we hand-off the portable to a randomly chosen adjacent
    // Cell (or agent).
    muse::AgentID receiverAgentID = 0; // random neighbor. 
    const int Change[4] = {-1, -Y, Y, 1};
    // Compute index into the Change array
    int index = state->getIndex();
    // Determine the receiver neighbor
    int new_index = (index + 1) % 4;
    // Update the destination agent.
    receiverAgentID = getAgentID() + Change[index];
    state->setIndex(new_index);
    // Handle wrap around cases.
    if (receiverAgentID < 0) {
        receiverAgentID += X * Y;
    }
    if (receiverAgentID >= (X * Y)) {
        receiverAgentID -= X * Y;
    }
    ASSERT((receiverAgentID >= 0) && (receiverAgentID < X * Y));
    // Create Portable and schedule -- note here we intentionally do
    // not use the helper schedulePortable() method. Furthermore,
    // hand-offs happen almost instantly. So the time increment here
    // is very small.
    const muse::Time receiveTime = getTime() + 0.125;
    PCS_Event* e = PCS_Event::create(receiverAgentID, receiveTime,
                                   move_timeStamp, next_call_timeStamp,
                                   complete_call_timeStamp, MOVE_IN,
                                   event.getPortableID());
    return e;
}

PCS_Event*
PCS_Agent::nextCall(const PCS_Event& event) {
    PCS_State * const state = dynamic_cast<PCS_State*> (getState());
    // The three random number generators used below.
    std::poisson_distribution<uint> duration_distrib(callDurationMean);
    std::exponential_distribution<double> move_distrib(1.0 / moveIntervalMean);
    std::exponential_distribution<double> interval_distrib(1.0 / callIntervalMean);
    // Extract timestamps from events (they will change below).
    muse::Time complete_call_timeStamp = event.getCompletionTimeStamp();
    muse::Time next_call_timeStamp     = event.getNextCallTimeStamp();
    muse::Time move_timeStamp          = event.getMoveTimeStamp();
    // Track a call was attempted.
    state->setCallAttempts(state->getCallAttempts() + 1);
    // See if this call can be received based on channel availability
    if (state->getIdleChannels() == 0) {
        // Channels are busy and so the call is counted as a blocked call.
        state->setBlockedChannels(state->getBlockedChannels() + 1);
        // Next call timestamp of the portable is incremented.
        const uint interCallDelay = interval_distrib(generator);
        next_call_timeStamp = getTime() + lookAhead + interCallDelay;
    } else {
        state->setIdleChannels(state->getIdleChannels() - 1);
        // Complete call timestamp of the portable is incremented.
        const uint callDuration = lookAhead + duration_distrib(generator);
        complete_call_timeStamp = getTime() + callDuration;
        // Now we need a loop to skip logically "busy" calls -- that
        // is until next_call_time is not the minimum of the actions
        // to be performed.
        do {
            // Update the next time a call will be received for this portable
            const uint interCallDelay = lookAhead + interval_distrib(generator);
            next_call_timeStamp += interCallDelay;
            if (next_call_timeStamp <= complete_call_timeStamp) {
                // The next call was logically set to be before end of
                // current call. So track that call as busy.
                state->setBusyLineCalls(state->getBusyLineCalls() + 1);
            }
        } while (TIME_EQUALS(next_call_timeStamp,
                             minTimeStamp(complete_call_timeStamp,
                                          next_call_timeStamp,move_timeStamp)));
    }
    // Schedule event to change state of this portable based on
    // minimum of the 3 timestamps.
    return getNextEvent(event.getPortableID(), complete_call_timeStamp,
                        next_call_timeStamp, move_timeStamp);
}

// This method is synoymous to the "StartUp" method in Carother's
// MASOCTS paper.
void
PCS_Agent::initialize() throw (std::exception) {
    // Random number generators for the 2 parameters in each portable
    std::exponential_distribution<double> move_distrib(1.0 /moveIntervalMean);
    std::exponential_distribution<double> interval_distrib(1.0 / callIntervalMean);
    /* We generate N Portables (i.e., PCS_Events) with random receive
     * times to self and initialize the timestamp fields by
     * exponentials with independent means. The
     * callCompletionTimeStamp is initialized to infinity because the
     * assumption is there are no calls at the start of the
     * simulation.
     */
    for (int i = 0, portableID = N * getAgentID(); i < N; i++, portableID++) {
        // Generate initial parameter settings based on
        // distributions. Note that as per the paper -- "We assume
        // that no calls are initially in progress"
        muse::Time complete_call_timeStamp = TIME_INFINITY;
        uint next_call_timeStamp     = lookAhead + interval_distrib(generator);
        uint move_timeStamp          = lookAhead + move_distrib(generator);
        // Schedule event to change state of this portable based on
        // minimum of the 3 timestamps.
        PCS_Event* e = getNextEvent(portableID, complete_call_timeStamp,
                                   next_call_timeStamp, move_timeStamp);
        scheduleEvent(e);
    }
}

void
PCS_Agent::simGranularity() {
    for (size_t i = 0; (i < granularity); i++) {
        double sum = 0;
        for (size_t delay = 0; (delay < 25L); delay++) {
            sum += std::sin(0.5);
        }
    }
}

void
PCS_Agent::processEvent(PCS_Event* nextEvent) {
    ASSERT(nextEvent != NULL);    
    PCS_Event* delEvent = NULL;
    // The loop below handles multiple operations that may have to
    // occur at the same time step. In this case multiple calls to
    // different helper methods occurr. Note that the loop is
    // structured under the assumption that the getAction() method
    // prefers move operations last (as move requires a small delta
    // change because MUSE does not allow zero-delay events)
    while (nextEvent->getReceiveTime() <= getTime()) {
        switch (nextEvent->getMethod()) {
        case NEXT_CALL:
            nextEvent = nextCall(*nextEvent);
            break;
        case MOVE_IN:
            nextEvent = moveIn(*nextEvent);
            break;
        case MOVE_OUT:
            nextEvent = moveOut(*nextEvent);
            break;
        case COMPLETE_CALL:
            nextEvent = completionCall(*nextEvent);
            break;
        default:
            std::cerr << "Unhandled method type encountered in PCS_Agent.cpp\n";
            ASSERT(false);  // abort simulation.
        }
        // Delete unscheduled event from previous iteration (if any)
        if (delEvent != NULL) {
            PCS_Event::deallocate(delEvent);
        }
        // Track event to be deleted if needed
        delEvent = nextEvent;
    }
    ASSERT(nextEvent != NULL);
    scheduleEvent(nextEvent);
}

void
PCS_Agent::executeTask(const muse::EventContainer* events) {
    for (auto it = events->begin(); it != events->end(); it++) {
        simGranularity();
        PCS_Event* const current_event = dynamic_cast<PCS_Event*>(*it);
        processEvent(current_event);
    }
} // End executeTask

void
PCS_Agent::finalize() {
    PCS_State* const state = dynamic_cast<PCS_State*> (getState());    
    // Print overall statistics upon finalization
    oss << "Cell[" << getAgentID() << "]: " << state->to_string() << std::endl;
}

#endif 
