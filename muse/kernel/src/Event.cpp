#ifndef _MUSE__EVENT_CPP
#define	_MUSE__EVENT_CPP
#include "Event.h"

using namespace muse;

Event::Event(const AgentID & senderID,const AgentID & receiverID,
               const Time & sentTime, const Time & receiveTime):
senderAgentID(senderID), receiverAgentID(receiverID),
        sentTime(sentTime), receiveTime(receiveTime), sign(true), referenceCount(0)
{}

const AgentID& 
Event::getSenderAgentID(){
    return senderAgentID;
}//end getSenderAgentID

const AgentID & 
Event::getReceiverAgentID(){
    return receiverAgentID;
}//end getReceiverAgentID

const Time & 
Event::getSentTime(){
    return sentTime;
}//end getSentTime

const Time & 
Event::getReceiveTime(){
    return receiveTime;
}//end getReceiveTime

void
Event::decreaseReference(){
    if (referenceCount > 0) referenceCount--;
    if (referenceCount == 0) {
        std::cout << "Getting deleted: " << this << std::endl;
        delete this;
    }
}//end decreaseReference

void
Event::increaseReference(){
    //std::cout << "Increasing ref" << std::endl;
    referenceCount+=1;
}//end increaseReference

void
Event::deleteEvent(){
    std::cout << "Getting deleted: " << this << std::endl;
    delete this;
}

void
Event::makeAntiMessage(){ sign=false;}

int
Event::getReferenceCount(){ return referenceCount;}

bool
Event::getSign(){
    return sign;
}
Event::~Event(){}

//void 
//Event::printInfo(){
//    cout << "Event Info" << endl;
//    cout << "Sender Agent: " << getSenderAgentID() << endl;
//    cout << "Receiver Agent: " << getReceiverAgentID() << endl;
//    cout << "Sent Time: " << getSentTime() << endl;
//    cout << "Receive Time: " << getReceiveTime() << endl;
//}
#endif

