#ifndef AGENTPQ_CPP
#define AGENTPQ_CPP

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Meseret Gebre          gebremr@muohio.edu
//          Dhananjai M. Rao       raodm@muohio.edu
//          Alex Chernyakhovsky    alex@searums.org
//
//---------------------------------------------------------------------------

#include "AgentPQ.h"
#include "BinaryHeapWrapper.h"

using namespace muse;

//ctor
AgentPQ::AgentPQ() : EventQueue("FibonacciHeap"), m_size(0) {}

//dtor
AgentPQ::~AgentPQ()            {
    if (m_size == 0)
	return;

    int idx = m_roots.size();
    while (idx-- != 0){
	if (m_roots[idx] != 0){
	    m_roots[idx]->destroy();
	    delete m_roots[idx];
	}//end if
    }//end while
}

//node methods below
void
AgentPQ::node::destroy()             {
    int idx = m_children.size();
    while (idx-- != 0){
	if (m_children[idx] != 0){
	    m_children[idx]->destroy();
	    delete m_children[idx];
	}//end if
    }//end while
}

AgentPQ::node*
AgentPQ::node::join(node* tree)      {
    tree->m_index  = m_children.size();
    tree->m_parent = this;
    m_children.push_back(tree);
    m_lost_child = 0;
    return this;
}

void
AgentPQ::node::cut(node* child)      {
    unsigned int index = child->m_index;
    if (m_children.size() > index + 1){
	m_children[index] = m_children[m_children.size() - 1];
	m_children[index]->m_index = index;
    }//end if
    m_children.pop_back();
    ++m_lost_child;
}

//fibonacci heap helper methods below
void
AgentPQ::add_root(node* n)           {
    unsigned rank = n->rank();
    if (m_roots.size() <= rank){
	while (m_roots.size() < rank){
	    m_roots.push_back(0);
	}//end while
	m_roots.push_back(n);
	n->clear();

    }else if (m_roots[rank] == 0){
	m_roots[rank] = n;
	n->clear();

    }else{
	node* r = m_roots[rank];
	m_roots[rank] = 0;
	if (compare(n->data(), r->data())){
	    n->clear();
	    add_root(r->join(n));
	}else{
	    r->clear();
	    add_root(n->join(r));
	}
    }//end if-else-ladder
}//end add_root

void
AgentPQ::find_min() const            {
    if (m_size == 0){
	m_min = 0;
    }else{
	std::vector<node*>::const_iterator end = m_roots.end();
	std::vector<node*>::const_iterator it  = m_roots.begin();
	while (*it == 0){
	    ++it;
	}//end while
	m_min = *it;

	for (++it; it != end; ++it){
	    if (*it != 0 && (m_min->data()->getTopTime() >= (*it)->data()->getTopTime()  )){
		m_min = *it;
	    }//end if
	}//end for
    }//end if-else
}

void
AgentPQ::cut(node* n)                {
    node* p = n->parent();
    p->cut(n);
    if (p->is_root()){
	m_roots[p->rank() + 1] = 0;
	add_root(p);
    }
    else if (p->lost_child() == 2){
	cut(p);
	add_root(p);
    }
}

//fibonacci heap modifying methods
void
AgentPQ::update(pointer n, Time old_top_time) {
    if (n->data()->getTopTime() > old_top_time) {
        // Means we need to push the data towards leafs
        decrease(n, n->data());
    }else if (n->data()->getTopTime() < old_top_time) {
        // Means we need to move up the heap towards root
        increase(n, n->data());
    }
}

void
AgentPQ::increase(pointer n, Agent* data) {
    n->data(data);
    if (!n->is_root() && compare(n->parent()->data(), n->data())){
	cut(n);
	add_root(n);
    }
    m_min = 0;
}

void
AgentPQ::decrease(pointer n, Agent* data) {
    n->data(data);
    std::vector<node*>::const_iterator it = n->begin();
    std::vector<node*>::const_iterator end = n->end();
    for (; it != end; ++it)
	if (compare(n->data(), (*it)->data())){
	    if (n->is_root()){
		m_roots[n->rank()] = 0;
	    }else{
		cut(n);
	    }
	    for (it = n->begin(); it != end; ++it){
		add_root(*it);
	    }
	    n->remove_all();

	    add_root(n);
	    break;
	}
    m_min = 0;
}

//void
//FibonacciHeap::remove(pointer n)           {
//  if (n->is_root())
//    m_roots[n->rank()] = 0;
//  else
//    cut(n);
//
//  std::vector<node*>::const_iterator it = n->begin();
//  std::vector<node*>::const_iterator end = n->end();
//  for (it = n->begin(); it != end; ++it){
//    add_root(*it);
//  }
//
//  m_min = 0;
//  --m_size;
//  delete n;
//}

//fibonacci heap basic interface
Agent*
AgentPQ::top() {
    if (m_min == 0) {
	find_min();
    }
    return m_min->data();
}

//void
//FibonacciHeap::pop() {
//  if (m_min == 0){
//    find_min();
//  }
//
//  node* del = m_min;
//  m_roots[m_min->rank()] = 0;
//
//  std::vector<node*>::const_iterator end = del->end();
//
//  for (std::vector<node*>::const_iterator it = del->begin(); it != end; ++it){
//    add_root(*it);
//  }
//  --m_size;
//
//  delete del;
//  m_min = 0;
//}

void*
AgentPQ::addAgent(Agent* data) {
    node* n = new node(data);
    ++m_size;
    add_root(n);

    m_min = 0;
    return n;
}

muse::Event*
AgentPQ::front() {
    muse::Event* retVal = NULL;
    if (!empty()) {
        retVal = top()->eventPQ->top();
    }
    return retVal;
}

void
AgentPQ::dequeueNextAgentEvents(muse::EventContainer& events) {
    if (!empty()) {
        muse::Agent *agent = top();
        agent->getNextEvents(events);
        ASSERT(!events.empty());
        pointer ptr = reinterpret_cast<pointer>(agent->fibHeapPtr);
        update(ptr, agent->oldTopTime);
        agent->oldTopTime = agent->getTopTime();
    }
}

void
AgentPQ::enqueue(muse::Agent* agent, muse::Event* event) {
    agent->eventPQ->push(event);
    pointer ptr = reinterpret_cast<pointer>(agent->fibHeapPtr);
    update(ptr, agent->oldTopTime);
    agent->oldTopTime = agent->getTopTime();
}

void
AgentPQ::enqueue(muse::Agent* agent, muse::EventContainer& events) {
    const Time oldTopTime = agent->oldTopTime;
    agent->eventPQ->push(events);
    pointer ptr = reinterpret_cast<pointer>(agent->fibHeapPtr);
    update(ptr, oldTopTime);
    agent->oldTopTime = agent->getTopTime();
}

int
AgentPQ::eraseAfter(muse::Agent* dest, const muse::AgentID sender,
                    const muse::Time sentTime) {
    const Time oldTopTime = dest->oldTopTime;    
    int numRemoved = dest->eventPQ->removeFutureEvents(sender, sentTime);
    pointer ptr = reinterpret_cast<pointer>(dest->fibHeapPtr);
    update(ptr, oldTopTime);
    dest->oldTopTime = dest->getTopTime();
    return numRemoved;
}

void
AgentPQ::reportStats(std::ostream& os) {
    UNUSED_PARAM(os);
    // No statistics are currently reported.
}

#endif