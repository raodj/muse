#ifndef DATA_TYPES_H
#define DATA_TYPES_H

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
// Authors: Meseret Gebre       gebremr@muohio.edu
//          Dhananjai M. Rao    raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include "Utilities.h"
#include <queue>
#include <set>
#include <map>
using namespace std;

BEGIN_NAMESPACE(muse);

// Forward declaration to define EventContainer
class Event;
class Agent;

/** The EventContainer.
    
    Holds a collection of events, this is basically a set of
    Events. Always use the EventContainer type, because the
    underlying implamentation could change.
*/
typedef std::set<Event*> EventContainer;

/** The AgentID struct.
    
    This is used to store the id of an agent. muse uses this AgentID
    struct to make sure the client know exactly what the param of a
    method is asking for. For more details check out the method
    Agent::getAgentID().  @see getAgentID()
    
*/
typedef unsigned int AgentID; 

typedef std::set<AgentID*> AgentIDContainer;
 
/** The Stream struct.
    
    @todo not sure if this will remain a struct or become a
    class. More investigation is required.
    
    @todo move to new header Stream.h, will be a class
*/
typedef struct StreamType{
    std::istream is;
    std::ostream os;
} Stream;


typedef double Time;

/** The SimulatorID struct.
 * This is used to store the id of a simulator(Singleton). muse uses this SimulatorID struct to make sure the client
 * know exactly what the param of a method is asking for. For more details check out the method getSimulatorID().
 * @see getSimulatorID()
 *
 */
typedef unsigned int SimulatorID;

/** The AgentContainer.
 * Holds a collection of agents, this is basically a set of Agents. Always
 * use the AgentContainer type, because the underlying implamentation could change.
 *
 */
typedef set<Agent*> AgentContainer;

END_NAMESPACE(muse);

#endif
