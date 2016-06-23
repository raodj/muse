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
#include "TwoTierHeapEventQueue.h"
#include "Scheduler.h"
#include "Agent.h"

class TestAgent : public muse::Agent {
public:
     TestAgent(int id) : muse::Agent(id, new muse::State()) {}
     void initialize() throw (std::exception) {}
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

void addEvents(TestScheduler& sched, const std::vector<int>& agentTimeList) {
    // Assume that agentTimeList has pairs of integers in the form
    // <agentID, recvTime>
    for (size_t i = 0; (i < agentTimeList.size()); i += 2) {
        muse::AgentID id(agentTimeList[i]);
        muse::Time time(agentTimeList[i + 1]);
        muse::Event *testEvent1 = muse::Event::create(id, time);
        sched.scheduleEvent(testEvent1);
    }
}

void addAgents(TestScheduler& sched, const std::vector<int>& agentIDList) {
    for (int id : agentIDList) {
        TestAgent*  agent = new TestAgent(id);
        sched.addAgentToScheduler(agent);    
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
    sched.prettyPrint(std::cout);

    return 0;
}

