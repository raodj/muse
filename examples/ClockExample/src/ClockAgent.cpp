#ifndef _CLOCKEVENT_CPP
#define	_CLOCKEVENT_CPP

#include "ClockAgent.h"
#include "ClockEvent.h"
#include <iostream>

using namespace std;

ClockAgent::ClockAgent(AgentID& id, ClockState* state) : Agent(id,state) {}

void
ClockAgent::initialize() throw (std::exception){
    cout << "Clock Agent INITIALIZE" << endl;
    //here i will set the motion of scheduling event for the first hour passage.
    //For this example we will assume that every FIVE time steps an hour has stopped
    //Therefore we will schedule the first event for Time 5.
    ClockEvent * e = new ClockEvent(this->getAgentID(),this->getAgentID(),
                                        Time(0), Time(5));
    this->scheduleEvent(e);
}//end initialize

void
ClockAgent::executeTask(const EventContainer* events){
    //when we receive a ClockEvent this means an hour has passed
    //print out our current LVT
    //and print HOUR has passed message.
    //lastly, schedule an event to myself for the next hour.
    cout << "HOUR PASSED @ LVT: " << this->LVT << endl;
    
    //here we are going through the EventContainer.
    //for the clock example we should only have one event per "HOUR" == (every 5 time steps)
    EventContainer::const_iterator it;
    for (it=events->begin(); it != events->end(); ++it ){
        ClockEvent * e = new ClockEvent(this->getAgentID(),this->getAgentID(),
                                        (*it)->getReceiveTime(), (*it)->getReceiveTime()+Time(5));

        
        this->scheduleEvent(e);
        //we update our state here
        ((ClockState*)myState)->hour = (*it)->getReceiveTime();
    }//end for

}//end executeTask

void
ClockAgent::finalize() {
    //cleanup and shutdown
    cout << "Clock Agent FINALIZE" << endl;

    cout << "\n\nClock Agent Input queue INFO" << endl;

    cout << "Clock Agent InputQueue Size: " << inputQueue.size() << endl;
    list<Event*>::iterator iit;
    for (iit=inputQueue.begin(); iit != inputQueue.end(); ++iit ){
        cout << "Event sent to agent: " << (*iit)->getReceiverAgentID() << endl;
        cout << "Event sent time: " << (*iit)->getSentTime() <<"\n\n" <<endl;
    }//end for

    cout << "\n\nClock Agent State INFO" << endl;

    cout << "Clock Agent StateQueue Size: " << stateQueue.size() << endl;
    list<State*>::iterator it;
    for (it=stateQueue.begin(); it != stateQueue.end(); ++it ){
        ClockState* cs = dynamic_cast<ClockState*>(*it);
        if (cs == NULL){
            cout << "CS is null" << endl;
        }else{
            cout << "State HOUR: " << cs->hour << endl;
        }
    }//end for

    cout << "\n\nClock Agent Output queue INFO" << endl;

    cout << "Clock Agent OutputQueue Size: " << outputQueue.size() << endl;
    list<Event*>::iterator eit;
    for (eit=outputQueue.begin(); eit != outputQueue.end(); ++eit ){
        cout << "Event sent to agent: " << (*eit)->getReceiverAgentID() << endl;
        cout << "Event sent time: " << (*eit)->getSentTime()<<endl;
        cout << "Event recv time: " << (*eit)->getReceiveTime() <<"\n" <<endl;
    }//end for
}//end finalize
#endif	/* _CLOCKEVENT_H */

