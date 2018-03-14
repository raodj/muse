#ifndef BAD_MT_Q_H
#define BAD_MT_Q_H

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
// Authors: Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include <mutex>
#include "ThreeTierHeapEventQueue.h"
#include "Agent.h"
#include "DataTypes.h"
#include "HashMap.h"

BEGIN_NAMESPACE(muse)

/** Extension of 3TierHeapQ that is naively thread safe - DO NOT ACTUALLY USE
  *
  * This queue holds a lock on every operation, making it primatively
  * thread safe for testing purposes but not practically useful in
  * anyway.
  * 
  * Note: This class overrides and locks on all operations exclusively for
  * testing purposes. The only methods that do anything useful are addAgent and
  * dequeueNextAgentEvents() in the cpp file
  */
class BadMTQ : public ThreeTierHeapEventQueue {
public:
	/** The constructor for the BadMTQ.

	The default (and only) constructor for this class.  The
	constructor does not have any specific task to perform other
	than set a suitable identifier in the base class.
	*/
	BadMTQ();

	/** The destructor.

	The destructor does not have any special tasks to perform.
	All the dynamic memory management is handled by the standard
	containers (namely std::vector) that is used internally by
	this class.
	*/
	~BadMTQ();

	/** Add/register an agent with the event queue.

	<p>This method implements the corresponding API method in the
	class.  Refer to the API documentation in the base class for
	intended functionality.</p>

	<p>This class uses the supplied agent pointer to setup the
	list of agents managed and scheduled by this class.</p>

	\param[in,out] agent A pointer to the agent to be registered.
	This value is not used.

	\return This method returns the iterator to the position of
	the agent in its internal vector as a cross-reference to be
	stored in an agent.
	*/
	virtual void* addAgent(Agent* agent) override;

	/** Remove/unregister an agent with the event queue.

	<p>This method implements the corresponding API method in the
	class.  Refer to the API documentation in the base class for
	intended functionality.</p>

	<p>This method removes all events scheduled for the specified
	agent in its internal data structures.</p>

	\param[in,out] agent A pointer to the agent whose events are
	to be removed from the vector managed by this class.
	*/
	void removeAgent(Agent* agent) override;

	/** Determine if the event queue is empty.

	This method implements the base class API to report if any
	events are pending to be processed in the event queue.

	\return This method returns true if the event queue of the
	top-agent is logically empty.
	*/
	virtual bool empty() override;

	/** Obtain pointer to the highest priority (lowest receive time)
	event.

	This method can be used to obtain a pointer to the highest
	priority event in this event queue, without de-queuing the
	event.

	\note The event returned by this method is not dequeued.

	\return A pointer to the next event to be processed.  If the
	queue is empty then this method returns NULL.
	*/
	virtual muse::Event* front() override;

	/** Method to obtain the next batch of events to be processed by
	one agent.

	<p>In MUSE agents are scheduled to process all events at a
	given simulation time.  The next concurrent events (i.e.,
	events with the same receive time) with the lowest time stamp
	are to be placed in the supplied event container.  The event
	container is then passed to the corresponding agent for
	further processing.</p>

	<p>This method essentially delegates the dequeue process to
	the agent with the next lowest timestamp.  Once the agent has
	been dequeued, this method fixes the heap by placing theAgentPointer
	top-agent in its appropriate location in the heap.</p>

	\param[out] events The event container in which the next set
	of concurrent events are to be placed.  Note that the order of
	concurrent events in the event container is unspecified.
	*/
	virtual void dequeueNextAgentEvents(muse::EventContainer& events) override;

	/** Enqueue a new event.

	This method must be used to enqueue/add an event to this event
	queue.  Once added the reference count on the event is
	increased.  This method adds the event to the specified agent.
	Next this method fixes the heap to ensure that the agent with
	the least-time-stamp is at the top of the heap.  This method
	essentially uses an internal helper method to accomplish its
	tasks.

	\param[in] agent The agent to which the event is to be
	scheduled.  This agent corresponds to the agent ID returned by
	event->getReceiverAgentID() method.

	\param[in] event The event to be enqueued.  This parameter can
	never be NULL.
	*/
	virtual void enqueue(muse::Agent* agent, muse::Event* event) override;

	/** Enqueue a batch of events.

	This method can be used to enqueue/add a batch of events to
	this event queue.  Once added the reference count on each one
	of the events is increased.  This method provides a convenient
	approach to enqueue a batch of events, particularly after a
	rollback.  Next this method fixes the heap to ensure that the
	agent with the least-time-stamp is at the top of the heap.
	This method uses an internal helper method to accomplish its
	tasks.

	\param[in] agent The agent to which the event is to be
	scheduled.  This agent corresponds to the agent ID returned by
	event->getReceiverAgentID() method.  Currently, this value is
	not used.

	\param[in] event The list of events to be enqueued.  This
	container can and will be be empty in certain situations.  The
	reference counts of the events in the container remains
	unmodified.  The list of events become part of the event
	queue.
	*/
	virtual void enqueue(muse::Agent* agent, muse::EventContainer& events);

	/** Dequeue all events sent by an agent after a given time.

	This method implements the base class API method.  This method
	can be used to remove/erase events sent by a given agent after
	a given simulation time.  This API is needed to cancel events
	during a rollback.  Next this method fixes the heap to ensure
	that the agent with the least-time-stamp is at the top of the
	heap.
AgentPointer
	\param[in] dest The agent whose currently scheduled events
	are to be checked and cleaned-up.  This agent must be a valid
	agent that has been registered/added to this event queue.  The
	pointer cannot be NULL.  This parameter is not used.

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
		const muse::Time sentTime) override;

protected:
	/** Enqueue a new event.

	This method must be used to enqueue/add an event to this event
	queue.  Once added the reference count on the event is
	increased.  This method adds the event to the specified agent.
	Next this method fixes the heap to ensure that the agent with
	the least-time-stamp is at the top of the heap.

	\param[in] agent The agent to which the event is to be
	scheduled.  This agent corresponds to the agent ID returned by
	event->getReceiverAgentID() method.

	\param[in] event The event to be enqueued.  This parameter can
	never be NULL.
	*/
	virtual void enqueueEvent(muse::Agent* agent, muse::Event* event) override;

	/** Convenience method to remove events.

	This is an internal convenience method that is used to remove
	the front (i.e., events with lowest timestamp) event list from this
	queue.
	*/
	void pop_front(muse::Agent* agent);

	/** Convenience method to obtain the top-most or front agent.

	This method can be used to obtain a pointer to the top/front
	agent -- that is, the agent with the lowest timestamp event to
	be scheduled next.

	\return A pointer to the top-most agent in this heap.
	*/
	muse::Agent* top();

	/** Convenience method to get the top-event time for a given
	agent.

	This method returns the top event time in the vector of events queue.
	If agent's vector of event queue is empty, then it returns infinity.

	\return The receive time of top event's recv time or
	TIME_INFINITY if vector is empty.
	*/
	muse::Time getTopTime(const muse::Agent* const agent);

	/** Comparator method to sort events in the heap.

	This is the comparator method that is passed to various
	standard C++ algorithms to organize events as a heap.  This
	comparator method gives first preference to receive time of
	events.  Tie between two events with the same recieve time is
	broken based on the receiver agent ID.

	\param[in] lhs The left-hand-side event to be used for
	comparison.  This parameter cannot be NULL.

	\param[in] rhs The right-hand-side event to be used for
	comparison. This parameter cannot be NULL.

	\return This method returns if lhs < rhs, i.e., the lhs event
	should be scheduled before the rhs event.
	*/
	bool compare(const Agent *lhs, const Agent * rhs) const;

	/** The getNextEvents method.

	This method is a helper that will grab the next set of events
	to be processed for a given agent.  This method is invoked in
	dequeueNextAgentEvents() method in this class.

	\param[out] container The reference of the container into
	which events should be added.
	*/
	void getNextEvents(Agent* agent, EventContainer& container);

	/** Update position of agent in the scheduler's heap.

	This is an internal helper method that is used to update the
	position of an agent in the scheduler's heap.  This method
	essentially performs sanity checks, uses the fixHeap() method
	to update position of the agent, and updates cross references
	for future use.

	\param[in] agent The agent whose position in the heap is to be
	updated.  This pointer cannot be NULL.

	\return This method returns the updated index position of the
	agent in agentList (the vector that serves as storage for the
	heap).
	*/
	size_t updateHeap(muse::Agent* agent);

	/** Fix-up the location of the agent in the heap.

	This method can be used to update the location of an agent in
	the heap.

	\note The implementation for this method has been heavily
	borrowed from libstdc++'s code base to ensure that heap
	updates are consistent with std::make_heap API.
	Unfortunately, this does imply that there is a chance this
	method may be incompatible with future versions.

	\param[in] currPos The current position of the agent in the
	heap whose position is to be updated.  This value is the index
	position of the agent in the agentList vector.

	\return This method returns the new position of the agent in
	the agentList vector.
	*/
	size_t fixHeap(size_t currPos);

	/** Helper method to reuse tier2 entries or create a new one.

	This method is a convenience method to recycle tier2 entry
	object is available.  If the recycle bin is empty, then this
	method creates a new object.

	\param[in] event The event to be used to initialize and to be
	contained in the newly created tier2 entry.

	\return A tier2 entry initialized and containing the given
	event.
	*/
	HOETier2Entry* makeTier2Entry(muse::Event* event);

private:
        /**
         * A mutex lock used to convert a non-thread safe queue into thread safe
         * 
         * This is a naive mutex because it implements the thread safe property
         * in a very ineffecient way and is used only for testing.
         */
	static std::recursive_mutex naiveMutex;
        
        /**
         * A mapping from agent id to agent so the queue can lock agents before
         * pulling their events
         */
        AgentIDAgentPointerMap agentMap;
};

END_NAMESPACE(muse)

#endif
