#ifndef MUSE_AGENT_H
#define MUSE_AGENT_H

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

#include <set>
#include <iostream>
#include <exception>
#include "DataTypes.h"
#include "Simulation.h"
#include "State.h"
#include <stack>

BEGIN_NAMESPACE(muse); //begin namespace declaration

class Event;

/** The base class for all agents in a simulation.
 
    <p>This class represents the base class from which all
    user-defined simulation agents must be derived. This class
    provides several methods that expose the core functionality of a
    muse simulation. In addition, the user is expected to implement
    some of the methods in this class. Refer to the documentation
    associated with each method for details on the functionality and
    requirements of the variuos methods in this class.</p>
        
    <p>To create an agent for your simulation, make sure you derive
    from this Agent Class, Also be sure to override all methods that
    are declared virtual.</p>
*/
class Agent {
public:
    /** The initialize method.
     
        This method is invoked whenever just when a simulation
        commences. The core simulation engine invokes this
        mehtod. This mehtod may perform any agent specific
        initialization activites that may be necessary. This includes
        opening new streams for performing I/O.
        
        @note Whenever a new agent is created, the dervied child class
        should override this method.

        @throw std::exception This method throws an exception when
        errors occur.
    */
    virtual void initialize() throw (std::exception);
        
    /** The finalize method.
     
        This method is invoked once the simulation has finished
        processing all events and is ending.  The core simulation
        engine invokes this method. This method may perform any final
        clean up or displaying results, basic I/O.
     
        @note Whenever a new agent is created, the derived child class
        should override this method.
    */
    virtual void finalize() throw ();
    
    /** The executeTask method.
     
        This method is invoked only when the agent has some events to
        process. The events that this agent must process at a given
        time step is all passed in one single shot to this method.
     
        @note That this method must not modify the events passed in.
        
        @note Whenever a new agent is implemented by extending this
        base class, the derived class should override this method.
        
        @param events The set of concurrent (events at the same time)
        events that this method should process.
        
        @see EventContainer
    */
    virtual void executeTask(const EventContainer * events);

    //------------Provided by muse----below-----------------//
    
    /** The scheduleEvent method.
     
        This method is used to schedule an event to be processed by any
        agent.  It takes the given Event and first checks if
        
        @param e this is an Event object and will not be modified by this
        method.
        
        @return bool this will let the client know if the operation was a
        success.
        
        @see Event()
    */
    bool scheduleEvent(Event * e);
    
    /** The getAgentID method returns a pointer to AgentID struct.
        
        This method is used to return a pointer to a AgentID struct
        which contains the id of the agent. Once pointer is obtained,
        simple using the command AgentID.id would extract the actual
        id, but for all purpose the client should have no reason to do
        this, unless the id is required for I/O purposes. All method
        that require will only take of type AgentID.
        
        Note: All agents must register with the muse::Simulator which
        in returns assigns an AgentID. This AgentID is what this
        method will be returning a pointer to.
        
        Note: AgentID that is returned should not be modified in anyway.
        
        @return AgentID 
        @see AgentID
        
    */
    const AgentID& getAgentID();
        
 
	
    /** The constructor.
        
        The ctor is not supposed to be overriden.
        @todo make sure that the above statement is a accurate statement.
    */
    explicit Agent(AgentID &, State &);
    
    /** The destructor.
     
        The dtor is not supposed to be overriden.
        @todo make sure that the above statement is a accurate statement.
    */
    ~Agent();
    
protected:
    /** The AgentID type myID.
     
        This is inialized when the agent is registered to a simulator.
        Only one way to access this, use the getAgentID method.
      
        @see getAgentID()
    */
    AgentID _myID;

	 /** The AgentID type myID.
     
        This is inialized when the agent is registered to a simulator.
        Only one way to access this, use the getAgentID method.
      
        @see getAgentID()
    */
	Time _LVT;

   
    stack<State*> stateStack;
    State _myState;

};
END_NAMESPACE(muse);//end namespace declaration
#endif
 
