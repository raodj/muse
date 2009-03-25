#---------------------------------------------------------------------------
#
# Copyright (c) Miami University, Oxford, OH.
# All rights reserved.
#
# Miami University (MU) makes no representations or warranties about
# the suitability of the software, either express or implied,
# including but not limited to the implied warranties of
# merchantability, fitness for a particular purpose, or
# non-infringement.  MU shall not be liable for any damages suffered
# by licensee as a result of using, result of using, modifying or
# distributing this software or its derivatives.
#
# By using or copying this Software, Licensee agrees to abide by the
# intellectual property laws, and all other applicable laws of the
# U.S., and the terms of this license.
#
# Authors: Meseret R. Gebre       meseret.gebre@gmail.com
#
#---------------------------------------------------------------------------

"""
This is the muse code generator templates module.
Here is where we define the templates that we use in the
muse code generator to create the Agents, Events and State header and cpp files.
"""

__author__="Meseret R. Gebre"
__date__="Mar 25, 2009"
__version__="0.1"

"""
Feel free to extend modify this any way you see fit. TAKE THIS CODE AS-IS.
Please leave everything above this line as is. If you modify or add anything
add what you modified or extended below.
"""


""" ***************BELOW ARE HEADER TEMPLATES********************* """

agent_header_template = """
#ifndef AGENT_NAME_HERE_H
#define AGENT_NAME_HERE_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.
    
    File: AGENT_NAME_HERE.h
    Author: your name

    ........give brief description of what this  agent does here.......
*/

#include "Agent.h"
#include "State.h"
#include "DataTypes.h"

using namespace muse;

class AGENT_NAME_HERE : public Agent {

public:
    AGENT_NAME_HERE(AgentID, State *);
    void initialize() throw (std::exception);
    void executeTask(const EventContainer* events);
    void finalize();
    ~AGENT_NAME_HERE();
};

#endif /* AGENT_NAME_HERE_H */
"""

state_header_template = """
#ifndef STATE_NAME_HERE_H
#define STATE_NAME_HERE_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: STATE_NAME_HERE.h
    Author: your name

    ........give brief description of what this  state contains here.......
*/

#include "State.h"
//#include "DataTypes.h"   //uncomment if you need muse data types

using namespace muse;
class STATE_NAME_HERE : public State {

public:
    STATE_NAME_HERE();
    State * getClone();
    ~STATE_NAME_HERE();
};

#endif /* STATE_NAME_HERE_H */
"""

event_header_template = """
#ifndef EVENT_NAME_HERE_H
#define EVENT_NAME_HERE_H

/*
    Auto generated with the muse code generator.
    Visit musesimulation.org for more info.

    File: EVENT_NAME_HERE.h
    Author: your name

    ........give brief description of what this  event means here.......
*/

#include "Event.h"
#include "DataTypes.h"

using namespace muse;

class EVENT_NAME_HERE : public Event {
public:
    EVENT_NAME_HERE();

    /** The getEventSize method.
        This is needed by muse kernel, do not erase.
        You can however do custom event size calculations.
    */
    inline int getEventSize() {return sizeof(EVENT_NAME_HERE);}

    ~EVENT_NAME_HERE();
};

#endif /* EVENT_NAME_HERE_H */
"""

""" ***************BELOW ARE CPP TEMPLATES********************* """

agent_cpp_template = """
#ifndef AGENT_NAME_HERE_CPP
#define AGENT_NAME_HERE_CPP

#include "AGENT_NAME_HERE.h"

AGENT_NAME_HERE::AGENT_NAME_HERE(AgentID id, State* state) : Agent(id,state){
    //insert ctor code here
}//end ctor

void
AGENT_NAME_HERE::initialize() throw (std::exception){
    //insert your init code here
}//end initialize

void
AGENT_NAME_HERE::executeTask(const EventContainer* events){
    //if you want you can uncomment the following code for event processing
    //EventContainer::const_iterator it = events->begin();
    //for (; it != events->end(); it++){
    //  Event * current_event = (*it);
    //....do something with current_event here......
    //}
}//end executeTask

void
AGENT_NAME_HERE::finalize(){
    //insert final code here
}//end finalize

AGENT_NAME_HERE::~AGENT_NAME_HERE(){
    //insert dtor code here
}//end dtor

#endif /* AGENT_NAME_HERE_CPP */
"""

state_cpp_template = """
#ifndef STATE_NAME_HERE_CPP
#define STATE_NAME_HERE_CPP

#include "STATE_NAME_HERE.h"

STATE_NAME_HERE::STATE_NAME_HERE(){
    //insert ctor code here
}//end ctor

State*
STATE_NAME_HERE::getClone(){
    //make sure you clone the state object correctly
    //check out muse examples for some hints
    //for primitive types shallow copy should work, however for pointers or
    //class you need to do a deep copy.
}//end getClone

STATE_NAME_HERE::~STATE_NAME_HERE(){
    //insert dtor code here
}//end dtor

#endif /* STATE_NAME_HERE_CPP */
"""

event_cpp_template = """
#ifndef EVENT_NAME_HERE_CPP
#define EVENT_NAME_HERE_CPP

#include "EVENT_NAME_HERE.h"

EVENT_NAME_HERE::EVENT_NAME_HERE(){
    //insert ctor code here
}//end ctor

EVENT_NAME_HERE::~EVENT_NAME_HERE(){
    //insert dtor code here
}//end dtor

#endif /* EVENT_NAME_HERE_CPP */
"""

""" ***************BELOW IS MAKEFILE TEMPLATE********************* """

makefile_template = """
AM_CXXFLAGS += -wd 810 -IMUSE_INCLUDE -I./agents/include -I./events/include -I./states/include

bin_PROGRAMS = PROJECT_NAME_HEREexec

PROJECT_NAME_HEREexec_LDFLAGS = -L../../kernel
PROJECT_NAME_HEREexec = $(STDCPP) -lmuse

PROJECT_NAME_HEREexec_SOURCES = \
	ALL_CLASSES_HERE
	PROJECT_NAME_HERESimulation.cpp

# end of Makefile.am
"""
