/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   TestThreeTierHeapEventQueue.h
 * Author: JuliusHigiro
 *
 * Created on June 20, 2016, 3:07 PM
 */

#ifndef TESTTHREETIERHEAPEVENTQUEUE_H
#define TESTTHREETIERHEAPEVENTQUEUE_H

#include <vector>
#include "TestEvent.h"

class TestTierThree {
private:
    std::vector<TestEvent>EventContainer;
    double receiveTime;
    int agentID;
public:
    TestTierThree() {}
    
    TestTierThree(const TestEvent event) {
        EventContainer.push_back(event);
        receiveTime = event.getReceiveTime();
        agentID = event.getAgentId();
    }
       
    void updateContainer(TestEvent event) {
           /*  std::cout <<"Updated: [Agent ID: " << event.getAgentId() << ", "
              << "Receive Time: " << event.getReceiveTime() 
              << "]"<< std::endl;*/
             EventContainer.push_back(event);
             for(int i = 0; i < EventContainer.size(); i++) {
                 std::cout << EventContainer[i] << std::endl;
             }
    }
    
    std::vector<TestEvent> getList() {
        return EventContainer;
    }
    
    double getReceiveTime() {
        return receiveTime;
    }
    
    int getAgentID() {
        return agentID;
    }
    
    
    bool operator == (const TestTierThree &rhs) {
        return (this->receiveTime == rhs.receiveTime);
    }
    
    void printEvents() {
        for (unsigned int i = 0; i < EventContainer.size(); i++) {
            
            std::cout <<"[Agent ID: " << EventContainer[i].getAgentId() << ", "
                      << "Receive Time: " << EventContainer[i].getReceiveTime()
                      << "]"<< std::endl;
        }
    }    
};

class TestThreeTierHeapEventQueue {
private:
    TestEvent event;
    std::vector<TestTierThree> tier2;
public:
    TestThreeTierHeapEventQueue();
    void schedule(TestEvent event);
    TestTierThree& getTier2Element(std::vector<TestTierThree>::size_type index);
    unsigned long tier2size();
    void TwoTierPrint();
    void ThreeTierPrint();
};

#endif /* TESTTHREETIERHEAPEVENTQUEUE_H */

