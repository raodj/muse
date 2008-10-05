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
#include "State.h"
#include "Time.h"
#include "DataTypes.h"

BEGIN_NAMESPACE(muse);
    
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
    virtual void executeTask(const EventContainer & events);

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
    bool scheduleEvent(const Event & e);
    
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
    const AgentID& getAgentID() const;
        
    /** The getSimulationTime method returns a pointer to Time, which
        contains NOW time by default.
     
        This method is used to return a pointer to a Time struct which
        contains the simulation time.  By default it will return the
        simulation time of NOW, optional params will be of type Enum
        TimeType.  Possible value to pass in are enum { NOW,
        START_TIME, END_TIME}; START_TIME is the time when the
        simualtion started, note does not have to be zero (0), but
        does have to be a positive. END_TIME is the time when the
        simulation will end, again this must be a positive value thats
        equal to or greater then the START_TIME.
        
        @param type this is an Enum called TimeType.
        
        @return Time *
        
        @see Time()
        
        @see TimeType
    */
    const Time* getSimulationTime(TimeType type) const;
        
    /** This method is used for dynamically creating agents. 
     
        This methos is used for creating agents during the
        simulation. When an agent is created via this method, it will
        automatically be registered to the Simulation Engine of its
        parent agent.  Parent Agent is the agent that is creating the
        agent. In the future, this restriction might be lefted, but
        for now it's what it is. Once this agent is registered, it
        losses all ties to the parent agent.
     
        Note: the Agent passed in will not be modified in anyway,
        except the assignment of an AgentID.
     
        @param agent this is of type Agent.
        
        @return bool true if the operation was successful
        
        @see Agent
        
        @todo remove the const from the param
    */
    bool createAgent(const Agent & agent);
        
    /** The migrateAgent method will move this agent to another
        Simulation Engine?
     
        @todo figure out if we want to use an AgentID or a SimulatorID. 
     
        @param ?
      
        @return bool true if the operation was a success.
        
        @see ?
    */
    bool migrateAgent(const AgentID & otherAgentID);
        
    /** The unregisterAgent method is used to remove this agent from
        the simulation.
        
        Not much to document about this method. Once this is called
        there will be no way to get this agent back, so use this with
        CAUTION! When this is called all info containing this agent
        will be removed, this includes State info, which could lead to
        a rollback.
        
        @return bool true if operation was successful.
        
        @todo figure out a way to make sure simulation will not go
        into rollback because of this call.
    */
    bool unregisterAgent();
        
    /** The createStream method, will return a pointer to a Stream type.
	
        Note sure what this Stream type will be yet???
	
        @todo fix this methods doc, when I have a better idea of the
        Stream type.
        
        @return Stream *
        @see Stream
    */
    Stream* createStream(const std::string& name,
			 const std::ios_base::openmode mode);
    
    /** The cloneState method will return a State * which will be a
        copy of this agents State.
        
        Since every agent has a State there is no need to pass in the
        State object to be cloned, this info can be derived from the
        Agent object. This method will return a State object.  This
        method will simply call State.getClone(), which should be
        implemented by the client.
        
        @todo remove the params because it is not needed.
        
        @todo figure out if we really need the return type to be const
        
        @see State()
    */
    const State* cloneState(const State & state) const;
    
    /** The serialize method is used to serialize this agent to a
        given output stream.
        
        @todo figure out exactly what it means to serial an agent????
        
        @param os this is of type std::ostream
        
        @return bool true if operation is successful
    */
    bool serialize(std::ostream & os) const;
	
    /** The constructor.
        
        The ctor is not supposed to be overriden.
        @todo make sure that the above statement is a accurate statement.
    */
    Agent() ;
    
    /** The destructor.
     
        The dtor is not supposed to be overriden.
        @todo make sure that the above statement is a accurate statement.
    */
    ~Agent();
    
private:
    /** The AgentID type myID.
     
        This is inialized when the agent is registered to a simulator.
        Only one way to access this, use the getAgentID method.
      
        @see getAgentID()
    */
    AgentID myID;
};

END_NAMESPACE(muse);

#endif
 
