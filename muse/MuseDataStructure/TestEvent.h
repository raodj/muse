/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TestEvent.h
 * Author: JuliusHigiro
 *
 * Created on June 20, 2016, 2:48 PM
 */

#ifndef TESTEVENT_H
#define TESTEVENT_H

class TestEvent {
    
private:
    int agentID;
    double receiveTime;
public:
    TestEvent(); 
    
    TestEvent(int agentID, double receiveTime); 
       
    int getAgentId() const; 
    
    double getReceiveTime() const; 
    
    inline static bool compEventProp(const TestEvent& lhs, const TestEvent& rhs);

    inline static bool compEvents(const TestEvent& lhs, const TestEvent& rhs);
};

std::ostream& operator<<(std::ostream& os, const TestEvent& event);


#endif /* TESTEVENT_H */

