#ifndef _PCSAgent_CPP
#define _PCSAgent_CPP

#include "PCSAgent.h"
#include "MTRandom.h"
#include "Simulation.h"
#include <cstdlib>
#include <cstdio>

PCSAgent::PCSAgent(muse::AgentID id, PCS_State* state, int x, int y, int n, int d,
        int num_channels, unsigned int call_interval_mean,
        unsigned int call_duration_mean, unsigned int move_interval_mean,
        int lookAhead, double selfEvents, size_t granularity) :
Agent(id, state), X(x), Y(y), N(n), Delay(d), lookAhead(lookAhead),
selfEvents(selfEvents), granularity(granularity), channels(num_channels),
callIntervalMean(call_interval_mean), callDurationMean(call_duration_mean),
moveIntervalMean(move_interval_mean), generator(id) {
    // Setup the random seed used for generating random delays.
    seed = id;
    myState = state;
    blocked_channels = call_attempts = hand_off_blocks = 0;
    
}

NextAction
PCSAgent::minTimeStamp(unsigned int completeCallTime, unsigned int nextCallTime, unsigned int moveCallTime) {
    
    NextAction action;
    if (completeCallTime <= nextCallTime) {
        action = (completeCallTime < moveCallTime) ? COMPLETECALL : MOVECALL;
    } else {
        action = (nextCallTime < moveCallTime) ? NEXTCALL : MOVECALL;
    }
    return action;
}

void
PCSAgent::completionCall(PCSEvent& event) {
    
    PCS_State * const pcs_state = static_cast<PCS_State*> (getState());
    unsigned int complete_call_timeStamp = infinity; // Reset to infinity.
    pcs_state->setIdleChannels(channels++); // Channel held by Portable is freed.
    unsigned int next_call_timeStamp = event.getNextCallTimeStamp();
    unsigned int move_timeStamp = event.getMoveTimeStamp();
    action = minTimeStamp(complete_call_timeStamp, next_call_timeStamp, move_timeStamp);
    switch (action) {
        case NEXTCALL: { method = NEXT_CALL; } break;
        case MOVECALL: { method = MOVE_OUT; } break;
        default:
            std::cerr << "Unhandled agent type encountered in PCSAgent.cpp\n";
            ASSERT(false);
    }
    // Make random receive time for the future.
    const int RndDelay = (Delay > 0) ? ((int) (rand_r(&seed) % Delay)) : 0;
    muse::Time receiveTime(getTime() + lookAhead + RndDelay);
    ASSERT(receiveTime > getTime());
    if (receiveTime < muse::Simulation::getSimulator()->getStopTime()) {
        PCSEvent * e = PCSEvent::create(getAgentID(), receiveTime,
                move_timeStamp, next_call_timeStamp,
                complete_call_timeStamp, method);
        scheduleEvent(e);
    }
}

void
PCSAgent::moveIn(PCSEvent& event, unsigned int moveDistrib) {
    
    PCS_State * const pcs_state = static_cast<PCS_State*> (getState());
    unsigned int complete_call_timeStamp = event.getCompletionTimeStamp();
    unsigned int next_call_timeStamp = event.getNextCallTimeStamp();
    unsigned int move_timeStamp = event.getMoveTimeStamp();
    move_timeStamp += moveDistrib;
    action = minTimeStamp(complete_call_timeStamp, next_call_timeStamp, move_timeStamp);
    // Check channel availability, if a call is in progress. 
    if (complete_call_timeStamp <= next_call_timeStamp) {
        if (pcs_state->getIdleChannels() == 0) { // All channels are busy.
            pcs_state->setHandOffBlocks(hand_off_blocks++);
            pcs_state->setBlockedChannels(blocked_channels++);
            // Call completion timestamp is reset to infinity
            complete_call_timeStamp = infinity;
        } else { // A channel is available and it is allocated.
            pcs_state->setIdleChannels(channels--);
            switch (action) {
                case COMPLETECALL: { method = COMPLETE_CALL; } break;
                case MOVECALL: { method = MOVE_OUT; } break;
                default:
                    std::cerr << "Unhandled action type encountered in PCSAgent.cpp\n";
                    ASSERT(false);
            }
        }
    } else { // No call is in progress.
        switch (action) {
            case NEXTCALL: { method = NEXT_CALL; } break;
            case MOVECALL: { method = MOVE_OUT; } break;
            default:
                std::cerr << "Unhandled action type encountered in PCSAgent.cpp\n";
                ASSERT(false);
        }
    }
    
    // Make random receive time for the future.
    const int RndDelay = (Delay > 0) ? ((int) (rand_r(&seed) % Delay)) : 0;
    muse::Time receiveTime(getTime() + lookAhead + RndDelay);
    ASSERT(receiveTime > getTime());
    if (receiveTime < muse::Simulation::getSimulator()->getStopTime()) {
        PCSEvent * e = PCSEvent::create(getAgentID(), receiveTime,
                move_timeStamp, next_call_timeStamp,
                complete_call_timeStamp, method);
        scheduleEvent(e);
    }
}

void
PCSAgent::moveOut(PCSEvent& event) {
    
    PCS_State * const pcs_state = static_cast<PCS_State*> (getState());
    unsigned int complete_call_timeStamp = event.getCompletionTimeStamp();
    unsigned int next_call_timeStamp = event.getNextCallTimeStamp();
    unsigned int move_timeStamp = event.getMoveTimeStamp();
    method = MOVE_IN;
    // Check if call is in progress and free channel.
    if (complete_call_timeStamp <= next_call_timeStamp) {
        pcs_state->setIdleChannels(channels++);
    }
    // First make a random receive time for the future
    const int RndDelay = (Delay > 0) ? ((int) (rand_r(&seed) % Delay)) : 0;
    muse::Time receiveTime(getTime() + lookAhead + RndDelay);
    int receiverAgentID = 0; // random neighbor. 
    // Create and send Portable to random neighbor.
    if (receiveTime < muse::Simulation::getSimulator()->getStopTime()) {
        // Schedule event to different agent.  We do this with
        // equal probability to choose 1 of 4 neighbours.
        const int Change[4] = {-1, -Y, Y, 1};
        // Compute index into the Change array
        int index = pcs_state->getIndex();
        // Determine the receiver neighbor
        int new_index = (index + 1) % 4;
        // Update the destination agent.
        receiverAgentID = getAgentID() + Change[index];
        pcs_state->setIndex(new_index);
        // Handle wrap around cases.
        if (receiverAgentID < 0) {
            receiverAgentID += X * Y;
        }
        if (receiverAgentID >= (X * Y)) {
            receiverAgentID -= X * Y;
        }
    }
    ASSERT((receiverAgentID >= 0) && (receiverAgentID < X * Y));
    // Create Portable and schedule
    PCSEvent * e = PCSEvent::create(receiverAgentID, receiveTime,
            move_timeStamp, next_call_timeStamp,
            complete_call_timeStamp, method);
    scheduleEvent(e);
}

void
PCSAgent::nextCall(PCSEvent& event,
        unsigned int durationDistrib,
        unsigned int intervalDistrib) {

    PCS_State * const pcs_state = static_cast<PCS_State*> (getState());
    unsigned int complete_call_timeStamp = event.getCompletionTimeStamp();
    unsigned int next_call_timeStamp = event.getNextCallTimeStamp();
    unsigned int move_timeStamp = event.getMoveTimeStamp();
    pcs_state->setCallAttempts(call_attempts++);
    if (pcs_state->getIdleChannels() == 0) {
        // Channels are busy and so the call is counted as a blocked call.
        pcs_state->setBlockedChannels(blocked_channels++);
        // Next call timestamp of the portable is incremented.
        next_call_timeStamp += intervalDistrib;
        method = (next_call_timeStamp < move_timeStamp)? NEXT_CALL : MOVE_OUT;
    } else { // Channel is available and so the call is allocated.
        pcs_state->setIdleChannels(channels--);
        // Complete call timestamp of the portable is incremented.
        complete_call_timeStamp += durationDistrib;
        method = (complete_call_timeStamp < move_timeStamp)? COMPLETE_CALL : MOVE_OUT;
    }
    
    // Make random receive time for the future.
    const int RndDelay = (Delay > 0) ? ((int) (rand_r(&seed) % Delay)) : 0;
    muse::Time receiveTime(getTime() + lookAhead + RndDelay);
    ASSERT(receiveTime > getTime());
    if (receiveTime < muse::Simulation::getSimulator()->getStopTime()) {
        PCSEvent * e = PCSEvent::create(getAgentID(), receiveTime,
                move_timeStamp, next_call_timeStamp,
                complete_call_timeStamp, method);
        scheduleEvent(e);
    }
}

void
PCSAgent::initialize() throw (std::exception) {
    
    std::poisson_distribution<unsigned int> duration_distrib(callDurationMean);
    std::poisson_distribution<unsigned int> move_distrib(moveIntervalMean);
    std::poisson_distribution<unsigned int> interval_distrib(callIntervalMean);
    /* We generate N PCSEvents/Portables with random receive times to self 
     * and initialize the timestamp fields by exponentials with independent
     * means. The callCompletionTimeStamp is initialized to infinity because the
     * assumption is there are no calls at the start of the simulation.
     */
    PCS_State * const pcs_state = static_cast<PCS_State*> (getState());
    pcs_state->setIdleChannels(channels);
    pcs_state->setBlockedChannels(blocked_channels);
    pcs_state->setCallAttempts(call_attempts);
    pcs_state->setHandOffBlocks(hand_off_blocks);
    for (int i = 0; i < N; i++) {
        unsigned int complete_call_timeStamp = duration_distrib(generator);
        unsigned int next_call_timeStamp = interval_distrib(generator);
        unsigned int move_timeStamp = move_distrib(generator);
        action = minTimeStamp(complete_call_timeStamp, next_call_timeStamp, move_timeStamp);
        switch (action) {
            case NEXTCALL: { method = NEXT_CALL; } break;
            case MOVECALL: { method = MOVE_OUT; } break;
            case COMPLETECALL: {
                if (pcs_state->getIdleChannels() > 0) {
                    pcs_state->setIdleChannels(channels--);
                    method = COMPLETE_CALL;
                } else {
                    pcs_state->setBlockedChannels(blocked_channels++);
                    method = (next_call_timeStamp < move_timeStamp)? NEXT_CALL : MOVE_OUT;
                }
            } break;
            default:
                std::cerr << "Unhandled action type encountered in PCSAgent.cpp\n";
                ASSERT(false);
        }
        const int RndDelay = (Delay > 0) ? ((int) (rand_r(&seed) % Delay)) : 0;
        muse::Time receive(getTime() + 1 + RndDelay);
        ASSERT(receive > getTime());
        if (receive < muse::Simulation::getSimulator()->getStopTime()) {
            PCSEvent * e = PCSEvent::create(getAgentID(), receive,
                    move_timeStamp, next_call_timeStamp,
                    complete_call_timeStamp, method);
            scheduleEvent(e);
        }
    }
}

void
PCSAgent::simGranularity() {
    
    for (size_t i = 0; (i < granularity); i++) {
        double sum = 0;
        for (size_t delay = 0; (delay < 25L); delay++) {
            sum += std::sin(0.5);
        }
    }
}

void
PCSAgent::executeTask(const muse::EventContainer* events) {
    
    std::poisson_distribution<unsigned int> duration_distrib(callDurationMean);
    std::poisson_distribution<unsigned int> move_distrib(moveIntervalMean);
    std::poisson_distribution<unsigned int> interval_distrib(callIntervalMean);
    for (auto it = events->begin(); it != events->end(); it++) {
        simGranularity();
        PCSEvent * const current_event = static_cast<PCSEvent*> ((*it));
        switch (current_event->getMethod()) {
            case NEXT_CALL: {
                nextCall(*current_event,
                        duration_distrib(generator),
                        interval_distrib(generator));
            } break;
            case MOVE_IN: {
                moveIn(*current_event,
                        move_distrib(generator));
            } break;
            case MOVE_OUT: { moveOut(*current_event); } break;
            case COMPLETE_CALL: { completionCall(*current_event); } break;
            default: {
                std::cerr << "Unhandled method type encountered in PCSAgent.cpp\n";
                assert(0);
            }
        }
    }
} // End executeTask

void
PCSAgent::finalize() {}

#endif 


