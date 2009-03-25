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
#ifndef __AGENT_NAME_HERE_H
#define __AGENT_NAME_HERE_H

/*
    Auto generated with the muse code generator. Visit musesimulation.org for more info.
    Author: your name
*/

#include "Agent.h"
#include "State.h"

class AGENT_NAME_HERE : public Agent {

};

#endif __AGENT_NAME_HERE_H
"""

state_header_template = """
#ifndef __STATE_NAME_HERE_H
#define __STATE_NAME_HERE_H

/*
    Auto generated with the muse code generator. Visit musesimulation.org for more info.
    Author: your name
*/

#include "State.h"

class STATE_NAME_HERE : public State {

};

#endif __STATE_NAME_HERE_H
"""

event_header_template = """
#ifndef __EVENT_NAME_HERE_H
#define __EVENT_NAME_HERE_H

/*
    Auto generated with the muse code generator. Visit musesimulation.org for more info.
    Author: your name
*/

#include "Event.h"

class EVENT_NAME_HERE : public Event {

};

#endif __EVENT_NAME_HERE_H
"""

""" ***************BELOW ARE CPP TEMPLATES********************* """

agent_cpp_template = """ """

state_cpp_template = """ """

event_cpp_template = """ """