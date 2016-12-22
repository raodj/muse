/* 
 * File:   PCSAgent.h
 * Author: Julius Higiro
 *
 * Created on August 31, 2016, 10:08 AM
 */

#ifndef PCSAGENT_H
#define PCSAGENT_H

#include <limits>
#include <memory>
#include <random>
#include "Agent.h"
#include "PCS_State.h"
#include "PCSEvent.h"

class PCSAgent : public muse::Agent {
public:
    PCSAgent(muse::AgentID, PCS_State*, int x, int y, int n, int d,
            int num_channels, unsigned int call_interval_mean,
            unsigned int call_duration_mean, unsigned int move_interval_mean,
            int lookAhead = 1, double selfEvents = 0.0, size_t granularity = 0);
    /**
     * The initialize method for this object is called only once when the
     * PCSAgent/Cell object is created. The PCSAgent/Cell initializes
     * PCSEvents/Portables and their corresponding timestamps
     * (call completion, next call and move timestamps). Initially, calls are
     * not in progress and the call completion timestamp is set to infinity.
     * The minimum of the three timestamps is used to determine the type and
     * the destination of the PCSEvent/Portable.
     */
    void initialize() throw (std::exception);
    
    /**
     * This method is called each time the PCSAgent/Cell receives
     * PCSEvent/Portable that it must process. The PCSAgent/Cell determines
     * the method to be invoked (nextCall, completionCall, moveIn, or moveOut)
     * and the subsequent actions that are taken based on the timestamps. 
     */
    void executeTask(const muse::EventContainer* events);
    
    /**
     * This method is called once just before the simulation 
     * completes.
     */    
    void finalize();
    
    /**
     * The availability of channels is determined. If channels are not available,
     * the call is recorded as a blocked call. The next call timestamp of the 
     * PCSEvent is incremented. If channels are available, the channel is
     * allocated to the PCSEvent/Portable. The call completion timestamp of the
     * PCSEvent is incremented. The minimum of the three timestamp fields is
     * computed.
     */
    void 
    nextCall(PCSEvent& event, unsigned int durationDistrib, 
            unsigned int intervalDistrib);
    
    /**
     * The move timestamp of the PCSEvent is incremented. The availability of
     * current PCSEvent/Portable is determined - call in progress? If call is in
     * progress the channel availability determined. If channel available, the
     * call is allocated and the minimum of the three timestamp fields is 
     * computed. If channel not available, the call is labeled a blocked call
     * (hand-off block). The call completion timestamp is reset to infinity and
     * the minimum of the three timestamp fields is computed. 
     */
    void 
    moveIn(PCSEvent& event, unsigned int moveDistrib);
    
    /**
     * The method is only invoked only when a call is in progress and a
     * PCSEvent/Portable is moving between PCSAgents/Cells. The PCSAgent/Cell
     * makes available the channel currently being used by the PCSEvent/Portable
     * that is departing. Next, the PCSAgent/Cell sends the PCSEvent/Portable to
     * another PCSAgent/Cell at the move timestamp. 
     */
    void
    moveOut(PCSEvent& event);
    
    /**
     * The call completion timestamp of the PCSEvent is reset to infinity.
     * The channel held by the PCSEvent/Portable is made available.
     * The minimum of the three timestamp fields is computed.
     */
    void
    completionCall(PCSEvent& event);
    
    /**
     * Computes the minimum time stamp and returns the next action
     */
    NextAction
    minTimeStamp(unsigned int completeCallTime, unsigned int nextCallTime, unsigned int moveCallTime);
    
protected:
    /** Simulate some granularity (i.e., CPU usage) for the event.

        This method merely runs a loop generating random numbers to
        simulate some processing done for each event.  Currently, the
        granularity is a fixed value, set when an agent is created.
     */
    void simGranularity();
    
private:
    int X, Y, N, Delay;

    /** Fixed lookahead virtual time delay for generating events.
        
        The virtual time events generated by this agent have this
        fixed look ahead virtual time value added to them.
     */
    int lookAhead;

    /** Fraction of events that the agent should schedule to itself.

        This value indicates the probability that this agent should
        schedule events to itself.  This value is in the range 0.0 to
        0.99 and is used with the boolean expression ((rand() % 1000)
        / 1000.0) < selfEvents.  If the boolean expression returns
        true, then this agent schedule events to itself.  Note that at
        a given virtual time, the maximum number of self-events is
        limited by events * N.
     */
    double selfEvents;

    /** The random seed value that is used to generate random delays.

        Each agent uses a custom random seed to ensure that the events
        generated by it have some degree of repeatability between
        runs. Otherwise the types of events generated between runs
        will be vastly different impacting results of tests.  In most
        cases it would be best to set the \c --delay parameter to 0 to
        ensure repeatable results.
     */
    unsigned int seed;

    /** A user-specified granularity -- that is, the number of loops
        to run for each event to add some load/granularity for each
        event.

        This instance variable is set to indicate the number of loops
        or iterations that must be executed by the agent to simulate
        some granularity, i.e., CPU used per event.
     */
    size_t granularity;
    
    /** The maximum number of channels that will assigned to each agent.
    */
    int channels;
    
    /** The average call duration time.
    */
    unsigned int callIntervalMean;
    
    /** The average call completion time. 
    */
    unsigned int callDurationMean;
    
    /** The average move time.
    */
    unsigned int moveIntervalMean;
     
    /** The numeric limit that is used to reset the call completion timestamp
     *  to infinity (the max value of an int).
     */
    unsigned int infinity = std::numeric_limits<int>::max();
    
    /** The engine that generates pseudo-random numbers.
     */
    std::mt19937 generator;
    
    /** The action to be taken which is dependent on the minimum timestamp.
     */
    NextAction action;
    
    /** The method the Agent/Cell invokes. 
     */
    Method method;
    
    /** The number of blocked calls that occur because channels
     *  are not available.
     */
    unsigned int blocked_channels;
    
    /** The number of calls attempted.
     */
    unsigned int call_attempts;
    
    /** The number of blocked calls that occur because channels
     *  are not available when engaged PCSEvent/Portables are in transit to 
     *  a new destination PCSAgent/Cell. 
     */
    unsigned int hand_off_blocks;
    
    /** The number of channels available
     * 
     */
    unsigned int idle_channels;
    
};

#endif /* PCSAGENT_H */

