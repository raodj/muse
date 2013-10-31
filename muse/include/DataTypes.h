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
#include <vector>
#include <iostream>

using namespace std;

BEGIN_NAMESPACE(muse)

// Forward declaration to define EventContainer
class Event;
class Agent;
class SimStream;

/** The EventContainer.
    
    Holds a collection of events, this is basically a set of
    Events. Always use the EventContainer type, because the
    underlying implamentation could change.
*/
typedef std::vector<Event*> EventContainer;

/** The AgentID type.
    
    This is used to store the id of an agent. MUSE uses this AgentID
    struct to make sure the client know exactly what the param of a
    method is asking for. For more details check out the method
    Agent::getAgentID().
    \see getAgentID()
    
*/
typedef int AgentID; 

/** The Time type.
    MUSE uses this Time type to make
    sure the client know exactly what the param of a method is asking
    for. 
 
 */
typedef double  Time;

/** The SimulatorID type.  This is used to store the id of a
 * simulator(Singleton). MUSE uses this SimulatorID type to make
 * sure the client know exactly what the param of a method is asking
 * for. For more details check out the method getSimulatorID().
 \see getSimulatorID()
 *
 */
typedef unsigned int SimulatorID;

/** The AgentContainer.
 * Holds a collection of agents, this is basically a set of Agents. Always
 * use the AgentContainer type, because the underlying implamentation could change.
 *
 */
typedef vector<Agent*> AgentContainer;

/** The SimStreamContainer of type vector.
    This is a container for storing pointers to
    SimStream derived objects.
 */
typedef vector<SimStream*> SimStreamContainer;


/** \def INFINITY
    
\brief A #define for virtual time corresponding to infinity.

This define provides a more human readable time corresponding to
infinity that is used for gvt computations.  This is also the
default value to which time instance variables are initialized.
*/
#define TIME_INFINITY muse::Time(1e30)




END_NAMESPACE(muse)

#endif
