#ifndef _PHOLDAgent_CPP
#define	_PHOLDAgent_CPP

/** The P-HOLD Simulation.
    BY: Meseret R. Gebre         meseret.gebre@muohio.edu

    Used Variables
    **************************
    X = Number of agents per row
    Y = Number of agents per column
    N = Number of initial events each agent starts with
    Delay = The Delay time for the receive time, this will be max range for a random from [0,1]

    MTRandom.h
    **************************
    The interface for MTRandom class is very simple. There are two static methods and two object methods.

    MTRandom::Rand() returns random generated number uniformly distributed in the range 0 - (2^32 - 1)
    MTRandom::RandDouble() returns MTRandom::Rand() and scales it to the range 0 - 1
    MTRandom::rand() is just like MTRandom::Rand(), but only for instances of MTRandom.
    MTRandom::randDouble() is just like MTRandom::RandDouble(), but only for instances of MTRandom.

    Constructing MTRandom
    ***************************
    MTRandom mt; //this version is default and will use a hardcoded seed.
    MTRandom mt(unsigned long seed); //this created with the given seed value.
*/


#include "PHOLDAgent.h"
#include "Event.h"
#include "MTRandom.h"

using namespace std;

PHOLDAgent::PHOLDAgent(AgentID id, State* state, int x, int y, int n, int d) : 
    Agent(id,state), X(x),Y(y), N(n), Delay(d){}

void
PHOLDAgent::initialize() throw (std::exception){
    //we generate N events with random receive times to self
    for (int i = 0; i < N; i++){
        Time  receive(1+(int)(MTRandom::RandDouble()*Delay));
        //cout << "INIT Random Receive Time: " <<receive <<endl;
        Event * e = new Event(getAgentID(),receive); 
        scheduleEvent(e);
    }
}//end initialize

void
PHOLDAgent::executeTask(const EventContainer* events){
   
    
    //for every event we get we send out one event
    for(int i=0;i < events->size(); i++){
        //first make a random receive time for the future
        Time receive(getTime()+(int)(MTRandom::RandDouble()*Delay));
        //now we need to choose which agent to send this event to.
        //we do this with equal probability for all 4 neighbours and send to one.
        AgentID receiverAgentID = -1;
        while (true){
            receiverAgentID = getAgentID();
            double r=MTRandom::RandDouble();
            if(r<0.25)      receiverAgentID--;
            else if(r<0.5)  receiverAgentID-=Y;
            else if(r<0.75) receiverAgentID+=Y;
            else            receiverAgentID++;
            
            //cout << "receiverAgentID before scaling " <<receiverAgentID <<endl;
            if(receiverAgentID < 0)receiverAgentID+=X*Y;
            if(receiverAgentID >=(int)(X*Y))receiverAgentID-=X*Y;
            //cout << "receiverAgentID beforeafter scaling " <<receiverAgentID <<endl;
            if (getAgentID() != receiverAgentID ) break;
        }
        //make event
        Event * e = new Event(receiverAgentID,receive);
        //schedule the event
        if( scheduleEvent(e))oss << "Agent ["<<getAgentID(); oss << "] Sent Event to Agent ["<<receiverAgentID <<
                                 "] for time [" <<receive <<"]"<<endl;
    }
}//end executeTask

void
PHOLDAgent::finalize() {}//end finalize

#endif 

