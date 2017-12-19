#ifndef PCS_AGENT_H
#define PCS_AGENT_H

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

/* 
 * File:   PCS_Agent.h
 * Author: Julius Higiro
 *
 * Created on August 31, 2016, 10:08 AM
 */

#include <random>
#include "Agent.h"
#include "PCS_State.h"
#include "PCS_Event.h"

/** Agent that models a PCS Cell. The Cell represents a cellular
    receiver/transmitter that has some fixed number of "channels"
    allocated to it and is the central entity (LP) object type for the
    simulation. A "channel" is a wireless channel via which a
    "portable" (or cellular device) can send/receive information from
    a Cell.  Portables are modeled using different values in the
    PCS_Event.
*/
class PCS_Agent : public muse::Agent {
public:
    /** The only constructor for this class.

        The constructor is relatively straightforward and merely
        initializes the instance variables to the corresponding
        parameter values.

        \param[in] id The ID to be set for the agent.  This value must
        be unique across the entire parallel simulation.

        \param[in] state The initial state to be set for this agent.

        \param[in] x The logical x-coordinate on the grid for this agent.

        \param[in] y The logical y-coordinate on the grid for this agent.

        \param[in] call_interval_mean The mean (in minutes, default is
        20 minutes) of the Exponential distribution used to determine
        the duration between 2 successive calls made by portables.
        Typically this value is the same for all agents to keep
        portable behaviors consistent.

        \param[in] call_duration_mean The mean (in minutes, default is
        2 minutes) duration of a phone call, with duration assumed to
        be Poisson distributed.  This mean value is used as the mean
        of a Poisson distribution that is used to determine duration
        of phone calls for a portable.  Typically this value is the
        same for all agents to keep portable behaviors consistent at
        each cell.

        \param[in] move_iterval_mean Mean (in minutes, default is 120
        minutes) of exponential distribution used to determine when a
        portable moves from one cell tower to another.  Typically this
        value is the same for all agents to keep portable behaviors
        consistent at each cell.

        \param[in] lookAhead An optional look ahead to be used when
        generating future time stamps.  This value is by default 1.
        It is used in scenarios when exponential distribution
        generates zero (from random number generation) but MUSE does
        not permit scheduling of events at the same time (i.e., MUSE
        does not support zero-length rollbacks).

        \param[in] granularity Used to add some CPU load for each
        event. Larger values add more load. Zero indicates no load at
        all.

        \param[in] printInfo Print information at the end of
        simulation in finalize method about the operations done by
        this agent.  This an application level message.
    */
    PCS_Agent(muse::AgentID id, PCS_State* state, int x, int y, int n,
              double call_interval_mean = 20.0, double call_duration_mean = 2.0,
              double move_interval_mean = 120.0,
              int lookAhead = 1, size_t granularity = 0,
              bool printInfo = true);

    /**
     * The initialize method for this object is called only once when the
     * PCS_Agent/Cell object is created. The PCS_Agent/Cell initializes
     * PCS_Events/Portables and their corresponding timestamps
     * (call completion, next call and move timestamps). Initially, calls are
     * not in progress and the call completion timestamp is set to infinity.
     * The minimum of the three timestamps is used to determine the type and
     * the destination of the PCS_Event/Portable.
     */
    void initialize() throw (std::exception) override;
    
    /**
     * This method is called each time the PCS_Agent/Cell receives
     * PCS_Event/Portable that it must process. The PCS_Agent/Cell determines
     * the method to be invoked (nextCall, completionCall, moveIn, or moveOut)
     * and the subsequent actions that are taken based on the timestamps. 
     */
    void executeTask(const muse::EventContainer& events) override;
 
    /**
     * This method is called once just before the simulation 
     * completes.
     */    
    void finalize() override;
    
    /**
     * The availability of channels is determined. If channels are not
     * available, the call is recorded as a blocked call. The next
     * call timestamp of the PCS_Event is incremented. If channels are
     * available, the channel is allocated to the
     * PCS_Event/Portable. The call completion timestamp of the
     * PCS_Event is incremented. The minimum of the three timestamp
     * fields is computed.
     */
    PCS_Event* nextCall(const PCS_Event& event);
    
    /**
     * Method to implement actions associated with a portable moving
     * into the range of this Cell.  The move timestamp of the
     * PCS_Event is incremented. The availability of current
     * PCS_Event/Portable is determined - call in progress? If call is
     * in progress the channel availability determined. If channel
     * available, the call is allocated and the minimum of the three
     * timestamp fields is computed. If channel not available, the
     * call is labeled a blocked call (hand-off block). The call
     * completion timestamp is reset to infinity and the minimum of
     * the three timestamp fields is computed.
     *
     * \param[in] event The current information associated with the
     * portable that has moved-in.
     *
     * \param[in] moveDistrib The delay to be added to the move time
     * in the event to determine the next time this portable will
     * move.
     */
    PCS_Event* moveIn(const PCS_Event& event);
    
    /**
     * The method is only invoked only when a call is in progress and
     * a PCS_Event/Portable is moving between PCS_Agents/Cells. The
     * PCS_Agent/Cell makes available the channel currently being used
     * by the PCS_Event/Portable that is departing. Next, the
     * PCS_Agent/Cell sends the PCS_Event/Portable to another
     * PCS_Agent/Cell at the move timestamp.
     */
    PCS_Event* moveOut(const PCS_Event& event);
    
    /**
     * Method to handle logical completion of a call by a portable.
     * As described in the paper -- "Upon invoking the CompletionCall
     * method, the Cell resets the call completion timestamp of the
     * Portable to 00 and frees the channel currently held by the
     * Portable. Again, the minimum of the three timestamp fields is
     * computed. The actions are identical to those described for the
     * Start Up method.  Note that when a call completes, a Portable
     * will generate its next incoming call."
     *
     * \param[in] event The portable (represented by the event) whose
     * state/method is to be updated by generating a new event.
     */
    PCS_Event* completionCall(const PCS_Event& event);
    
    /**
     * Returns the next action/state based on the minimum of the 3
     * time stamps specified to this method.
     */
    muse::Time minTimeStamp(muse::Time completeCallTime,
                            muse::Time nextCallTime,
                            muse::Time moveCallTime) const;

    /**
     * Returns the next action/state based on the minimum of the 3
     * time stamps specified to this method.
     */
    NextAction getAction(muse::Time completeCallTime,
                         muse::Time nextCallTime,
                         muse::Time moveCallTime) const;
    
protected:
    /** Simulate some granularity (i.e., CPU usage) for the event.

        This method merely runs a loop generating random numbers to
        simulate some processing done for each event.  Currently, the
        granularity is a fixed value, set when an agent is created.
     */
    void simGranularity();

    PCS_Event* getNextEvent(const uint portableID,
                           const muse::Time callCompleteTime,
                           const muse::Time nextCallTime,
                           const muse::Time moveTime) const;

    void processEvent(PCS_Event* event);

private:
    const int X, Y, N;
    
    /** Fixed lookahead virtual time delay for generating events.
        
        The virtual time events generated by this agent have this
        fixed look ahead virtual time value added to them.
     */
    const int lookAhead;

    /** The random seed value that is used to generate random delays.

        Each agent uses a custom random seed to ensure that the events
        generated by it have some degree of repeatability between
        runs. Otherwise the types of events generated between runs
        will be vastly different impacting results of tests.  In most
        cases it would be best to set the \c --delay parameter to 0 to
        ensure repeatable results.
     */
    uint seed;
    
    /** A user-specified granularity -- that is, the number of loops
        to run for each event to add some load/granularity for each
        event.

        This instance variable is set to indicate the number of loops
        or iterations that must be executed by the agent to simulate
        some granularity, i.e., CPU used per event.
     */
    const size_t granularity;
    
    /** The mean time between two successive calls to a portable
        associated with this Cell.  This value is essentially the mean
        of an expoential random distribution from where the next call
        timestamp value is determined.
    */
    const double callIntervalMean;
    
    /** The mean call completion time. This value essentially
        represents the mean used for a Poisson distribution used to
        generate the length/duration of a call to a portable.
    */
    const double callDurationMean;
    
    /** The mean move time.  This value essentially represents the
        mean used for an expoential random distribution used to
        generate the time when a portable will move to an adjacent
        cell.
    */
    const double moveIntervalMean;
     
    /** The numeric limit that is used to reset the call completion timestamp
     *  to infinity (the max value of an int).
     */
    muse::Time infinity = TIME_INFINITY;
        
    /** 
     * The engine that generates pseudo-random numbers that is used
     * only in the nextCall and initialize methods
     */
    std::mt19937 generator;

    /** 
     * The engine that generates pseudo-random numbers that is used
     * only in the moveIn and initialize methods.
     */
    std::mt19937 moveGenerator;

    /** A temporary event container that is used to fully-sort the
        events to ensure a total order as the PCS model is sensitive
        to the order of events.  The events in this list are sorted
        using the totalOrderCompare method below.
    */
    std::vector<PCS_Event*> fullySortedEvents;
    
    /** Event comparison function in the executeTaks method to enforce
        a total order on the events being processed as the behavior of
        PCS simulation is sensitive to the order of events.  This
        comparator is used on concurrent events (that is events with
        same receiveTime) at one agent (so all receiverAgentIDs are
        the same).

        \param[in] lhs The left-hand-side event for comparison.  The
        pointer cannot be NULL.

        \param[in] lhs The right-hand-side event for comparison.  The
        pointer cannot be NULL.
        
        \return This method returns true if lhs is less than rhs.
        That is, lhs should be processed before rhs.
    */
    static inline bool totalOrderCompare(const PCS_Event* const lhs,
                                         const PCS_Event* const rhs) {
        if (lhs->getSentTime() != rhs->getSentTime()) {
            return (lhs->getSentTime() < rhs->getSentTime());
        }
        // Sent times are the same. Sort next on the sender agent
        if (lhs->getSenderAgentID() != rhs->getSenderAgentID()) {
            return (lhs->getSenderAgentID() < rhs->getSenderAgentID());
        }
        // Next, sort based on the operation to be performed
        if (lhs->getMethod() != rhs->getMethod()) {
            return (lhs->getMethod() < rhs->getMethod());
        }
        // Finally sort on portable id
        return (lhs->getPortableID() < rhs->getPortableID());
    }

    /** Flag to enable/disable printing information at the end of
        simulation.

        This flag is used in the finalize method to control if message
        about the operations done by this agent is to be printed.
        This an application level message which can clutter outputs in
        performance benchmarking and can be suppressed via \c
        --no-info command-line argument.
    */
    const bool printInfo;
};

#endif /* PCS_AGENT_H */
