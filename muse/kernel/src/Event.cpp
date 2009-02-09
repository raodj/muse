#ifndef _MUSE__EVENT_CPP
#define	_MUSE__EVENT_CPP
#include "Event.h"
#include "Simulation.h"

using namespace muse;

Event::Event(const AgentID  senderID,const AgentID  receiverID,
               const Time  sentTime, const Time  receiveTime):
senderAgentID(senderID), receiverAgentID(receiverID),
        sentTime(sentTime), receiveTime(receiveTime), sign(true), referenceCount(0)
{}

const AgentID& 
Event::getSenderAgentID() const{
    return senderAgentID;
}//end getSenderAgentID

const AgentID & 
Event::getReceiverAgentID() const{
    return receiverAgentID;
}//end getReceiverAgentID

const Time & 
Event::getSentTime() const{
    return sentTime;
}//end getSentTime

const Time & 
Event::getReceiveTime() const{
    return receiveTime;
}//end getReceiveTime

void
Event::decreaseReference(){
    if (referenceCount > 0) referenceCount-=1;
    // cout << "\nRef Count: " << referenceCount << endl;
    if (referenceCount == 0) {
      //std::cout << "Getting deleted: " << this << std::endl;
    
    if ((Simulation::getSimulator())->isAgentLocal(senderAgentID) ){
      delete this;
    }else{
      //cout << "Event was not local: calling delete[]\n\n";
      delete []  ((char *) this);
    }
    }
}//end decreaseReference

void
Event::increaseReference(){
    //std::cout << "Increasing ref" << std::endl;
    referenceCount+=1;
}//end increaseReference



void
Event::makeAntiMessage(){ sign=false;}

int
Event::getReferenceCount(){ return referenceCount;}

bool
Event::getSign(){
    return sign;
}
Event::~Event(){}

ostream&
::operator<<(ostream& os, const muse::Event& event) {
    os << "Event[Sender=" << event.getSenderAgentID()   << ","
       << "receiver="     << event.getReceiverAgentID() << ","
       << "sentTime="     << event.getSentTime()        << ","
       << "recvTime="     << event.getReceiveTime()     << "]";
     return os;
}

// std::copy(list.begin(), list.end(), std::output_iterator<Event*>(std::cout));

//void 
//Event::printInfo(){
//    cout << "Event Info" << endl;
//    cout << "Sender Agent: " << getSenderAgentID() << endl;
//    cout << "Receiver Agent: " << getReceiverAgentID() << endl;
//    cout << "Sent Time: " << getSentTime() << endl;
//    cout << "Receive Time: " << getReceiveTime() << endl;
//}
#endif

