#ifndef MUSE_COMMUNICATOR_CPP
#define MUSE_COMMUNICATOR_CPP

#include "Communicator.h"
#include "Event.h"
#include "Agent.h"
#include <mpi.h>

using namespace muse;

Communicator::Communicator(){}

SimulatorID
Communicator::initialize(){
    int x=0;
    char **arg = new char*[1];
    arg[0] = "";
    MPI::Init(x,arg); //arguments are trash, quick hack to get init working
    //return rank;
    return MPI::COMM_WORLD.Get_rank();
}

void
Communicator::registerAgents(AgentContainer& allAgents){
    int  size = MPI::COMM_WORLD.Get_size();

    //if size == 1 : then we need not do this!
    if (size == 1) return;
    int  simulator_id = MPI::COMM_WORLD.Get_rank();

     if (simulator_id == ROOT_KERNEL){
        
            //first lets add root_kernel local agents to map!
            AgentContainer::iterator ac_it; //ac == AgentContainer
            for (ac_it=allAgents.begin(); ac_it != allAgents.end(); ++ac_it){
                 agentMap[(*ac_it)->getAgentID()] = ROOT_KERNEL;
            }//end for

            //here we collect all agent id from all other simulation kernels!!
            int done_count = 1;
            MPI::Status status;
            while ( !(done_count == size)){
                //probe for a message from another simulation kernel.
                MPI::COMM_WORLD.Probe(MPI_ANY_SOURCE, AGENT_LIST, status);
                //figure out the agent list size
                int agent_list_size = status.Get_count(MPI::UNSIGNED);
                unsigned int agentList[agent_list_size];
                //now receive the agent id flat list
                MPI::COMM_WORLD.Recv( &agentList ,agent_list_size, MPI::UNSIGNED , MPI_ANY_SOURCE , AGENT_LIST, status );
                //time to add this list to master AgentMap
                for(int i=0; i<agent_list_size; ++i){
                    agentMap[agentList[i]] = status.Get_source();
                }//end for 
                done_count++;
            }//end while

            //next chunk of code converts agentMap to a flat list for Bcasting!!
            map<AgentID, SimulatorID>::iterator it;
            int agentMap_size = agentMap.size()*2;
            unsigned int  agentMap_flat[agentMap_size];
            int counter=0;
            for (it=agentMap.begin(); it != agentMap.end(); ++it){
                ///cout << "Agent ID: " << it->first << " Registered to Kernel ID: " << it->second << endl;
                agentMap_flat[counter] = it->first;
                agentMap_flat[counter+1] = it->second;
                counter+=2;
            }//end for

            //before we bcast the agentMap, we must Bcast the size of the list!
            MPI::COMM_WORLD.Bcast(&agentMap_size, 1 ,MPI::INT, ROOT_KERNEL );
            //bcast here
            MPI::COMM_WORLD.Bcast(&agentMap_flat, agentMap_size,MPI::UNSIGNED, ROOT_KERNEL );

           
     }else{
              AgentID agentList[allAgents.size()];
              for (int i=0; i < allAgents.size(); ++i){
                 agentList[i] = allAgents[i]->getAgentID();//populate the flat list.
              }//end for
              //now send flat list across the wire (MPI)
              MPI::COMM_WORLD.Send(&agentList, allAgents.size(), MPI::UNSIGNED,ROOT_KERNEL,AGENT_LIST);
              //get the size of incoming agentMap list
              int agentMap_length;
              MPI::COMM_WORLD.Bcast(&agentMap_length, 1 ,MPI::INT, ROOT_KERNEL );

              //finally receive the complete agentMap flat list!!!
              unsigned int agentMap_flatList[agentMap_length];
              MPI::COMM_WORLD.Bcast(&agentMap_flatList, agentMap_length,MPI::UNSIGNED, ROOT_KERNEL );

              //use final flat list and populate local AgentMap list!!
              for (int i=0; i < agentMap_length; i+=2){
                  agentMap[agentMap_flatList[i]] = agentMap_flatList[i+1];
                  //cout << "Agent ID: " << agentMap_flatList[i] << " Registered to Kernel ID: " << agentMap_flatList[i+1] << endl;
              }//end for
             
     }//end if
}//end registerAgent method

void
Communicator::sendEvent(Event * e, int & event_size){
    //no good way to send objects via MPI
    //make Event to a char* aka ghetto hack :-)
     char* serialEvent = (char*)&e;
     MPI::COMM_WORLD.Isend(serialEvent, event_size, MPI::CHAR,agentMap[e->getReceiverAgentID()],EVENT);
     cout << "SENT an Event of size: " << event_size << endl;
}//end sendEvent

Event*
Communicator::receiveEvent(){
    MPI::Status status;
    if (MPI::COMM_WORLD.Iprobe(MPI_ANY_SOURCE, EVENT, status)){
        //figure out the agent list size
        int event_size = status.Get_count(MPI::CHAR);
        char incoming_event[event_size];
        MPI::COMM_WORLD.Irecv( incoming_event,event_size, MPI::CHAR , MPI_ANY_SOURCE , EVENT, status);
        Event* the_event = (Event*)incoming_event;
        cout << "RECEIVED an Event of size: " << event_size << endl;
        cout << "event for agent: " << the_event->getReceiverAgentID() << endl;
        cout << "event to  agent: " << the_event->getSenderAgentID() << endl;
        return the_event;
    }
    return NULL;
}//end receiveEvent

void
Communicator::finalize()
{
    MPI::Finalize();
}

Communicator::~Communicator(){}

#endif
