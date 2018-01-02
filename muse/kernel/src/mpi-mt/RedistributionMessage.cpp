#ifndef REDISTRIBUTION_MESSAGE_CPP
#define	REDISTRIBUTION_MESSAGE_CPP

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

#include "mpi-mt/RedistributionMessage.h"
#include "EventAdapter.h"

// Switch default namespace to streamline code
using namespace muse;

RedistributionMessage*
RedistributionMessage::create(const int numaID, const int numEntries,
                              const int entrySize, std::stack<char*>& src) {
    // First compute the message size.
    const int msgSize = sizeof(RedistributionMessage) +
        (sizeof(char*) * numEntries);
    // Allocate flat memory for the message.  Note here we
    // intentionally use new on purpose!
    char* memory = new char[msgSize];
    // Now use the flat memory to instantiate an object.
    RedistributionMessage* msg = new (memory)
        RedistributionMessage(numaID, numEntries, entrySize, msgSize);
    // Move numEntries worth of chunks from src into the message
    for (int i = 0; (i < numEntries); i++) {
        if (!src.empty()) {
            msg->chunks[i] = src.top();  // Copy the pointer
            src.pop();              // Remove copied value.
        } else {
            // No entries. So set pointers to NULL.
            msg->chunks[i] = NULL;
        }
    }
    // Now we have a message object to return/work-with
    return msg;
}

void
RedistributionMessage::addEntriesTo(std::stack<char*>& dest) {
    for (int i = 0; (i < entryCount); i++) {
        if (chunks[i] != NULL) {
            dest.push(chunks[i]);  // Add entry from message to stack.
            chunks[i] = NULL;      // Clear out pointers (just in-case)
        }
    }
}

RedistributionMessage::RedistributionMessage(const int numaID,
                                             const int numEntries,
                                             const int entrySize,
                                             const int msgSize) :
    muse::Event(-2, -2), msgSize(msgSize), numaID(numaID),
    entryCount(numEntries), entrySize(entrySize) {
    // Setup the sender to sentinel value for identification
    // of message in processIncomingEvents
    EventAdapter::setSenderAgentID(this, REDISTR_MSG_SENDER);    
}

void
RedistributionMessage::destroy(RedistributionMessage* msg) {
    ASSERT(msg != NULL);
    char* buffer = reinterpret_cast<char*>(msg);
    delete [] buffer;
}

#endif
