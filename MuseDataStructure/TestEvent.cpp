/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TestEvent.cpp
 * Author: JuliusHigiro
 *
 * Created on June 2, 2016, 2:36 PM
 */

#include <cstdlib>
#include <vector>
#include <iostream>
#include <ostream>
#include "TestEvent.h"

TestEvent::TestEvent() {
    agentID = 0;
    receiveTime = 0.0;
}
    
TestEvent::TestEvent(int agentID, double receiveTime) {
    this->agentID = agentID;
    this->receiveTime = receiveTime;
}
       
int TestEvent::getAgentId() const {return agentID; }
double TestEvent::getReceiveTime() const {return receiveTime; }

inline static bool compEventProp(const TestEvent& lhs, const TestEvent& rhs) {
    return lhs.getAgentId() == rhs.getAgentId() && 
    lhs.getReceiveTime() == rhs.getReceiveTime();
}   

inline static bool compEvents(const TestEvent& lhs, const TestEvent& rhs) {
    return lhs.getReceiveTime() < rhs.getReceiveTime();
}

std::ostream& operator<<(std::ostream& os, const TestEvent& event) {
        os << "[AgentID: " << event.getAgentId() << ", "
            << "ReceiveTime: " << event.getReceiveTime() << "]";
    return os;
}

