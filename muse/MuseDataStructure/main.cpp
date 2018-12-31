/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: Julius Higiro
 *
 * Created on June 23, 2016, 2:32 PM
 */

#include <cstdlib>
#include <vector>
#include "TwoTierHeapEventQueue.h"
#include "ThreeTierHeapEventQueue.h"
#include "Scheduler.h"
#include "Agent.h"
#include "BinaryHeap.h"
#include "BinaryHeapWrapper.h"
#include "Event.h"
#include "EventQueue.h"
#include "DataTypes.h"

class TestAgent : public muse::Agent {
public:
     TestAgent(int id) : muse::Agent(id, new muse::State()) {}
     void initialize() {}
     void executeTask(const muse::EventContainer* eventList) {}
     void finalize() {}
};

using namespace std;

class TestScheduler : public muse::Scheduler {
public:
    TestScheduler(const std::string& queueName) {
        char *argv[2];
        argv[0] = strdup("--scheduler-queue");
        argv[1] = strdup(queueName.c_str());
        int argc = 2;
        initialize(0, 1, argc, argv);
    }
};

void testThreeTierHeap(muse::ThreeTierHeapEventQueue& threeTier, const std::vector<int>& agentTimeList) {
    // Assume that agentTimeList has pairs of integers in the form
    // <agentID, recvTime>
    for (size_t i = 0; (i < agentTimeList.size()); i += 2) {
        muse::AgentID id(agentTimeList[i]);
        muse::Time time(agentTimeList[i + 1]);
        muse::Event *testEvent = muse::Event::create(id, time);
        threeTier.schedule(testEvent);     
    }   
     threeTier.placeOnHeap();
}
template <typename T>
void addListOfEventsToBinaryHeap(muse::BinaryHeap<T>& bHeap, const std::vector<int>& agentTimeList) {
    // Assume that agentTimeList has pairs of integers in the form
    // <agentID, recvTime>
    muse::EventContainer eventList;
    for (size_t i = 0; (i < agentTimeList.size()); i += 2) {
        muse::AgentID id(agentTimeList[i]);
        muse::Time time(agentTimeList[i + 1]);
        muse::Event *testEvent = muse::Event::create(id, time);
        eventList.push_back(testEvent);
    }
    bHeap.push(eventList);
}

template <typename T>
void addEventsToBinaryHeap(muse::BinaryHeap<T>& bHeap, const std::vector<int>& agentTimeList) {
    // Assume that agentTimeList has pairs of integers in the form
    // <agentID, recvTime>
    for (size_t i = 0; (i < agentTimeList.size()); i += 2) {
        muse::AgentID id(agentTimeList[i]);
        muse::Time time(agentTimeList[i + 1]);
        muse::Event *testEvent = muse::Event::create(id, time);
        bHeap.push(testEvent);
    }
}

void addAgents(TestScheduler& sched, const std::vector<int>& agentIDList) {
    for (int id : agentIDList) {
        TestAgent*  agent = new TestAgent(id);
        sched.addAgentToScheduler(agent);    
    }
}

void addEvents(TestScheduler& sched, const std::vector<int>& agentTimeList) {
    for (size_t i = 0; (i < agentTimeList.size()); i += 2) {
        muse::AgentID id(agentTimeList[i]);
        muse::Time time(agentTimeList[i + 1]);
        muse::Event *testEvent = muse::Event::create(id, time);
        sched.scheduleEvent(testEvent);
    }
}

/*
 * 
 */
int main(int argc, char** argv) {    
    // Create the test scheduler with the queue name as the parameter
    TestScheduler sched("2theap");
    // Create test agents
    addAgents(sched, {1, 2, 3});
    // Add some events
    addEvents(sched, {1, 1,   1, 3,   3, 2});
    // Print the scheduler queue to see if it looks correct
    //sched.prettyPrint(std::cout);
   
    muse::ThreeTierHeapEventQueue threeTierHeap;
    testThreeTierHeap(threeTierHeap, {1, 1,   1, 3,   3, 2});
    
    
    muse::BinaryHeap<muse::EventContainer> bHeap;   
    //addEventsToBinaryHeap(bHeap, {1, 1,   1, 3,   3, 2});
    addListOfEventsToBinaryHeap(bHeap, {1, 1,   1, 3,   1, 2,   2, 2,   3, 4,   1, 8,   2, 7});
    bHeap.print(std::cout);
    std::cout << "Size: " << bHeap.size() << std::endl;
    bHeap.pop();
    bHeap.pop();
    bHeap.pop();
    bHeap.print(std::cout);
    std::cout << "Size: " << bHeap.size() << std::endl;
 
    return 0;
}
