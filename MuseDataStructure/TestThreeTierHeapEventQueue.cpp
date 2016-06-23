/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#include <vector>
#include <iostream>
#include <algorithm>
#include "TestEvent.h"
#include "TestThreeTierHeapEventQueue.h"

TestThreeTierHeapEventQueue::TestThreeTierHeapEventQueue() {}

void TestThreeTierHeapEventQueue::schedule(TestEvent event) {
    
    auto check = std::find(tier2.begin(), tier2.end(), event);
    auto index = std::distance(tier2.begin(), check);
    if(check != tier2.end()) {
        TestTierThree current = tier2[index];
        current.updateContainer(event);
        /*
        std::cout <<"Updated: [Agent ID: " << event.getAgentId() << ", "
                  << "Receive Time: " << event.getReceiveTime() 
                  << "]"<< std::endl;*/
    }
    else {
        TestTierThree tierThree(event);
        tier2.push_back(tierThree);
    }
   
}

void
TestThreeTierHeapEventQueue::TwoTierPrint() {
    for(unsigned int i = 0; i < tier2.size(); i++) {
        std::cout <<"[Agent ID: " << tier2[i].getAgentID() << ", "
                  << "Receive Time: " << tier2[i].getReceiveTime() 
                  << "]";
    }
}

void
TestThreeTierHeapEventQueue::ThreeTierPrint() {
    TestTierThree tierThree;
   
    for(unsigned int i = 0; i < tier2.size(); i++) {
        std::cout << "Tier2 index: " << i << std::endl;
        tierThree = tier2[i];
        tierThree.printEvents();
    } 
}

int main(int argc, char** argv) {
   
    
    TestEvent event1(1, 7.0);
    TestEvent event2(2, 1.0);
    TestEvent event3(22, 1.0);
    TestEvent event4(3, 4.0);
    TestEvent event5(33, 4.0);
    TestEvent event6(222, 1.0);
    TestEvent event7(333, 4.0);
    
    TestTierThree tierThree; 
    TestThreeTierHeapEventQueue threetier;
    
    threetier.schedule(event1);
    threetier.schedule(event2);
    threetier.schedule(event3);
    threetier.schedule(event4);
    threetier.schedule(event5);
    threetier.schedule(event6);
    threetier.schedule(event7);
    
    //threetier.TwoTierPrint();
    //threetier.ThreeTierPrint();
    
   
    
  
    
    return 0;
}

