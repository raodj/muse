#ifndef _MUSE__EVENT_CPP
#define	_MUSE__EVENT_CPP
#include "Event.h"

using namespace muse;

Event::Event(const AgentID & senderID,const AgentID & receiverID,
               const Time & sentTime, const Time & receiveTime):
senderAgentID(senderID), receiverAgentID(receiverID),
        sentTime(sentTime), receiveTime(receiveTime)
{}

const AgentID& 
Event::getSenderAgentID(){
    return senderAgentID;
}

const AgentID & 
Event::getReceiverAgentID(){
    return receiverAgentID;
}

const Time & 
Event::getSentTime(){
    return sentTime;
}

const Time & 
Event::getReceiveTime(){
    return receiveTime;
}

//void 
//Event::printInfo(){
//    cout << "Event Info" << endl;
//    cout << "Sender Agent: " << getSenderAgentID() << endl;
//    cout << "Receiver Agent: " << getReceiverAgentID() << endl;
//    cout << "Sent Time: " << getSentTime() << endl;
//    cout << "Receive Time: " << getReceiveTime() << endl;
//}
#endif

