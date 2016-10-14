/* 
 * File:   AgentPQ.h
 * Author: meseret gebre - meseret.gebre@gmail.com
 *
 * This implementation is based from  Dietmar Kuehl's f_heap implementation.
 // --------------------------------------------------------------------------
 // This implementation is based on the description found in "Network Flow",
 // R.K.Ahuja, T.L.Magnanti, J.B.Orlin, Prentice Hall. The algorithmic stuff
 // should be basically identical to this description, however, the actual
 // representation is not.
 // --------------------------------------------------------------------------
 * Created on April 19, 2009, 3:08 AM
 */

#ifndef AGENTPQ_H
#define AGENTPQ_H

#include <functional>
#include <vector>
#include <iostream>
#include "EventQueue.h"
#include "BinaryHeapWrapper.h"

BEGIN_NAMESPACE(muse);

class AgentPQ : public EventQueue {
    
protected:
    //This is the node define. what the fibonacci heap will maintain.
    class node {
    public:

        node(Agent * data) : m_parent(0), m_lost_child(0), m_data(data) {
            m_children.reserve(8);
        }

        ~node() {
        }

        void destroy();
        node* join(node* tree); // add tree as a child
        void cut(node* child); // remove the child

        int lost_child() const {
            return m_lost_child;
        }

        void clear() {
            m_parent = 0;
            m_lost_child = 0;
        }

        int rank() const {
            return m_children.size();
        }

        bool is_root() const {
            return m_parent == 0;
        }

        node* parent() const {
            return m_parent;
        }

        void remove_all() {
            m_children.erase(m_children.begin(), m_children.end());
        }

        Agent* data() const {
            return m_data;
        }

        void data(Agent* data) {
            m_data = data;
        }

        std::vector<node*>::const_iterator begin() const {
            return m_children.begin();
        }

        std::vector<node*>::const_iterator end() const {
            return m_children.end();
        }

        void ppHelper(std::ostream& os, const std::string &indent) const {
            os << indent << ((is_root()) ? "*" : ">") << *data() << std::endl;
            for (size_t i = 0; (i < m_children.size()); i++) {
                if (m_children[i] != 0) {
                    m_children[i]->ppHelper(os, indent + "-");
                }
            }//end for
        }//end ppHelper

    private:
        int m_index; // index of the object in the parent's vector
        node* m_parent; // pointer to the parent node
        std::vector<node*> m_children;
        int m_lost_child;
        Agent* m_data;

        node(node const&);
        void operator=(node const&);
    };

protected:
    std::vector<node*> m_roots;
    int m_size;

public:
    typedef node* pointer;

    AgentPQ();
    ~AgentPQ();

    /** Add/register an agent with the event queue.

        This method implements the EventQueue API method that is
        invoked to add/register each agent that is present on the
        local Simulator/process.  This method creates a new Fibonacci
        node entry for the agent and adds it as the current root of
        the Fibonacci heap. The minimum value is reset to NULL, to
        ensure the minimum root is recomputed when needed.

        \param[in,out] agent A pointer to the agent to be registered.

        \return This method returns a pointer to the node that was
        created.  This node is never deleted and is strictly
        associated with the agent.
     */
    virtual void* addAgent(Agent* data);

    /** Remove/unregister an agent with the event queue.

        <p>This method implements the corresponding API method in the
        class.  Refer to the API documentation in the base class for
        intended functionality.</p>

        <p>This method removes all events scheduled for the specified
        agent in its internal data structures.</p>
        
        \param[in,out] agent A pointer to the agent whose events are
        to be removed from the heap managed by this class.
    */
    void removeAgent(muse::Agent* agent) override;

    /** Determine if the event queue is empty.

        This method implements the base class API to report if any
        events are pending to be processed in the event queue.

        \return This method returns true if the heap is logically
        empty.
     */
    virtual bool empty() {
        return (m_size == 0) || (top()->schedRef.eventPQ->empty());
    }

    /** Obtain pointer to the highest priority (lowest receive time)
        event.

        This method implements base class API method to return a
        pointer to the highest priority event in this event queue,
        without de-queuing the event.

        \note The event returned by this method is not dequeued.
        
        \return A pointer to the next event to be processed.  If the
        heap is empty then this method returns NULL.
     */
    virtual muse::Event* front();

    /** Method to obtain the next batch of events to be processed by
        one agent.

        In MUSE agents are scheduled to process all events at a given
        simulation time.  The next concurrent events (i.e., events
        with the same receive time) with the lowest time stamp are to
        be placed in the supplied event container.

        \param[out] events The event container in which the next set
        of concurrent events are to be placed.  Note that the order of
        concurrent events in the event container is unspecified.
     */
    virtual void dequeueNextAgentEvents(muse::EventContainer& events);

    /** Enqueue a new event.

        This method must be used to enqueue/add an event to this event
        queue.  Once added the reference count on the event is
        increased.

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.
        
        \param[in] event The event to be enqueued.  This parameter can
        never be NULL.
     */
    virtual void enqueue(muse::Agent* agent, muse::Event* event);

    /** Enqueue a batch of events.

        This method can be used to enqueue/add a batch of events to
        this event queue.  Once added the reference count on each one
        of the events is increased.  This method provides a convenient
        approach to enqueue a batch of events, particularly after a
        rollback.

        \param[in] agent The agent to which the event is to be
        scheduled.  This agent corresponds to the agent ID returned by
        event->getReceiverAgentID() method.

        \param[in] event The event to be enqueued.  This parameter can
        never be NULL.
     */
    virtual void enqueue(muse::Agent* agent, muse::EventContainer& events);

    /** Dequeue all events sent by an agent after a given time.

        This method can be used to remove/erase events sent by a given
        agent after a given simulation time.  This API is needed to
        cancel events during a rollback.

        \param[in] dest The agent whose currently secheduled events
        are to be checked and cleaned-up.  This agent must be a valid
        agent that has been registered/added to this event queue.  The
        pointer cannot be NULL.
        
        \param[in] sender The ID of the agent whose events have to be
        removed.  This agent must be a valid agent that has been
        registered/added to this event queue.  The pointer cannot be
        NULL.

        \param[in] sentTime The time from which the events are to be
        removed.  All events (including those sent at this time) sent
        by the sender agent are removed from this event queue.

        \return This method returns the number of events actually
        removed.
     */
    virtual int eraseAfter(muse::Agent* dest, const muse::AgentID sender,
            const muse::Time sentTime);

    Agent* top();
    void update(pointer n, double old_top_time);
    //void     remove(pointer);

    bool empty() const {
        return m_size == 0;
    }

    int size() const {
        return m_size;
    }

    virtual void prettyPrint(std::ostream& os) const {
        for (size_t i = 0; (i < m_roots.size()); i++) {
            if (m_roots[i] != 0) {
                m_roots[i]->ppHelper(os, "-");
            }
        }
    }

    /** Method to report aggregate statistics.

        This method is invoked at the end of simulation after all
        agents on this rank have been finalized.  This method is meant
        to report any aggregate statistics from this queue.
        Currently, this method does not write any specific statistics.

        \param[out] os The output stream to which the statistics are
        to be written.
     */
    virtual void reportStats(std::ostream& os);

protected:
    /** The getNextEvents method.

        This method is a helper that will grab the next set of events
        to be processed for a given agent.  This method is invoked in
        dequeueNextAgentEvents() method in this class.
		
        \param[out] container The reference of the container into
        which events should be added.        
    */
    void getNextEvents(Agent* agent, EventContainer& container);
    
private:
    void decrease(pointer, Agent*);
    void increase(pointer, Agent*);
    void add_root(node* n);
    void cut(node* n);
    void find_min() const;
    mutable node* m_min;

    inline muse::Time getTopTime(const Agent* const agent) const {
        return agent->schedRef.eventPQ->getTopTime();
    }
    
    inline bool compare(const Agent *lhs, const Agent * rhs) const {
        return (getTopTime(lhs) >= getTopTime(rhs));
    }

    AgentPQ(AgentPQ const&); // deliberately not implemented
    void operator=(AgentPQ const&); // deliberately not implemented
};


END_NAMESPACE(muse); //end namespace

#endif /* AGENTPQ_H */
