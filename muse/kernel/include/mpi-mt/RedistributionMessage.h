#ifndef REDISTRIBUTION_MESSAGE_H
#define	REDISTRIBUTION_MESSAGE_H

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
// Authors:  Dhananjai M. Rao       raodm@miamiOH.edu
//
//---------------------------------------------------------------------------

#include <stack>
#include "DataTypes.h"
#include "Event.h"

BEGIN_NAMESPACE(muse);

/** This is a sentinel value that is used to distinguish
    redistribution message from other incoming messages/events in
    MultiThreadedSimulation's processIncomingEvents method.  This
    value is set in the message's sender field in the constructor of
    RedistributionMessage
*/
constexpr int REDISTR_MSG_SENDER = -2;

/** Message to improve recycling unused memory chunks by
    redistributing them to various threads.
    
    <p><u>Problem</u>: When NUMA-aware event recycling is used in
    multi-threaded simulations, the size of unused event pool can be
    skewed -- that is, some threads end up having a large fraction of
    events that are not used.  Furthermore, the "NUMA Recycler %hits"
    statistic would degrade well below 90% to 70%.  This issue is most
    obvious when shared events are not used, but can occurr in even
    with shared events depending on the communication patterns.</p>

    <p><u>Solution:</u>: Now, the NumaMemoryManager has the ability to
    redistribute unused memory chunks, evenly to different threads.
    This message is used to send unused chunks to different threads
    using the normal inter-thread communication mechanism. At the end
    of each garbage collection cycle, the multi-threaded simulator
    calls the redistribute method in the NumaMemory manager (note that
    this happens on each and every thread).  The NumMemory manager
    checks to see if it has more recycled chunks than the NUMA-blocks
    it actually allocated.  If so, it redistributes the unused memory
    chunks evenly to all the threads using this message.</p>

    \see NumaMemoryManager::redistribute
    \see MultiThreadedSimulation::processIncomingEvents
*/
class RedistributionMessage : public muse::Event {
public:

    /** Primary method to create this message.

        This method allocates memory for the redistribution message as
        a flat array of bytes, consistent with the MUSE kernel
        requirements.  It then moves the first numEntries of pointers
        from the supplied stack into the message.

        \param[in] numaID The ID of the NUMA node on which all of the
        recycled memory blocks have been allocated.  This value is
        copied into the new message being created.

        \param[in] numEntries The number of entries from src to be
        moved into the newly created message.

        \param[in] entrySize The size (in bytes) of each memory chunk
        being moved from the specified stack.  Note that all of the
        memory chunks in the stack are assumed to be of the same size.

        \param[in,out] src The list of recycled memory blocks from
        where pointers are moved into this message.  Note that
        numEntries are popped off this stack.  So the stack is
        modified by this method.

        \return The newly created redistribution message with
        numEntries of recycled chunks removed from src.

        \see RedistributionMessage::destroy
    */
    static RedistributionMessage* create(const int numaID,
                                         const int numEntries,
                                         const int entrySize, 
                                         std::stack<char*>& src);

    /** Free memory used for this message.

        Since this message is created as a flat array of bytes (in
        create method), this method must be used to free the memory
        correctly.

        \param[in] msg The message to be deleted.
    */
    static void destroy(RedistributionMessage* msg);
    
    /** Add entries in this message to the specified stack.

        This is a convenience method that is used to add all the
        chunks in this message to the specified stack.  This method
        puses getEntryCount number of entries onto the stack.  Once
        pushed the entries in the message are cleared.

        \param[out] dest The stack onto which the chunks in this
        message are to be pushed.
    */
    void addEntriesTo(std::stack<char*>& dest);

    /** Get the NUMA node ID set in this message.

        \return This method returns the numaID set in this message
        when it was created.
    */
    inline int getNumaID() const { return numaID; }

    /** Get the number of recycled memory chunks in this message.

        \return This method returns the number of recycled chunks in
        this message.  This value is set when the message was created.
    */
    inline int getEntryCount() const { return entryCount; }

    /** Get the size (in bytes) of each recycled memory chunk.

        \return This method returns the size (in bytes) of each chunk
        in this message.  This value is set when the message was
        created.
    */
    inline int getEntrySize() const { return entrySize; }
    
    /** \brief Get the total size (in bytes) of this Event
        
        This method is used to determine the total size of each Event
        so that it can be properly sent across the wire by MPI.

        \return The total size of the event in bytes as required by
        MUSE kernel's API.
    */
    int getEventSize() const override { return msgSize; }
    
protected:
    /** The constructor.

        The constructor is intentionally protected to minimize the
        chances it is called directly.  Instead use the create method
        in this class to create messages.

        \param[in] numaID The ID of the NUMA node on which all of the
        recycled memory blocks have been allocated.  This value is
        copied into the new message being created.

        \param[in] numEntries The number of entries from src to be
        moved into the newly created message.

        \param[in] entrySize The size (in bytes) of each memory chunk
        being moved from the specified stack.  Note that all of the
        memory chunks in the stack are assumed to be of the same size.

        \param[in] msgSize The total size of this message in bytes.
    */
    RedistributionMessage(const int numaID, const int numEntries,
                          const int entrySize, const int msgSize);

    /** \brief The destructor.

        The destructor has been made protected to ensure that this
        message is never directly deleted.  Instead use destory()
        method in this class.
    */
    ~RedistributionMessage() {}

private:
    /** \brief Instance variable to hold the size of the message (in
        bytes).

        This instance variables holds the raw size (in bytes)
        of this message.  This information is used to dispatch this
        message to another process via a suitable MPI call.  The size
        value is set by the constructor and is never changed.
    */
    const int msgSize;

    /** The NUMA node ID associated with the memory chunks being
        recycled.
    */ 
    const int numaID;

    /** The number of memory chunks being redistributed via this
        message. This value indicates the number of pointers in the
        chunks array.
    */
    const int entryCount;

    /** The size of each chunk of memory in this message. */
    const int entrySize;
    
    /** \brief The variable length array of pointers to memory chunks
        being recycled.

        \note This instance variable must be at the end of this class
        definition.  Do not move it.
        
        This instance variable contains a variable number of entries
        depending on the number of memory chunks being redistributed.
        This vector is logically created when a suitable GVT message
        is instantiated and sufficient memory is allocated hold the
        data for this vector.
    */
    char* chunks[];    
};

END_NAMESPACE(muse);

#endif
