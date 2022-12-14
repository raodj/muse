/* 
 * File:   Location.h
 * Author: Julius Higiro
 *
 * Created on March 26, 2017, 4:26 PM
 */

#ifndef LOCATION_H
#define LOCATION_H

#include <limits>
#include <memory>
#include <random>
#include "Agent.h"

class Location : public muse::Agent {
public:
    
    Location(const muse::AgentID id);
    
    void initialize() override;
    
    void executeTask(const muse::EventContainer& events) override;
    /**
     * This method is called once just before the simulation 
     * completes.
     */
    void finalize() override;

protected:
    /** Simulate some granularity (i.e., CPU usage) for the event.

        This method merely runs a loop generating random numbers to
        simulate some processing done for each event.  Currently, the
        granularity is a fixed value, set when an location is created.
     */
    void simGranularity();

private:
    int X, Y, N, Delay;

    /** Fixed lookahead virtual time delay for generating events.
        
        The virtual time events generated by this location have this
        fixed look ahead virtual time value added to them.
     */
    int lookAhead;

    /** Fraction of events that the location should schedule to itself.

        This value indicates the probability that this location should
        schedule events to itself.  This value is in the range 0.0 to
        0.99 and is used with the boolean expression ((rand() % 1000)
        / 1000.0) < selfEvents.  If the boolean expression returns
        true, then this location schedule events to itself.  Note that at
        a given virtual time, the maximum number of self-events is
        limited by events * N.
     */
    double selfEvents;

    /** The random seed value that is used to generate random delays.

        Each location uses a custom random seed to ensure that the events
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
        or iterations that must be executed by the location to simulate
        some granularity, i.e., CPU used per event.
     */
    size_t granularity;

private:


};

#endif /* LOCATION_H */

