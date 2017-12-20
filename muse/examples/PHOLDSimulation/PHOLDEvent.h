#ifndef PHOLD_EVENT_H
#define PHOLD_EVENT_H

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

#include "Event.h"

/** A simple extension to muse::Event to enable different event
    lengths.

    This class represents all events in a PHOLD simulation.
*/
class PHOLDEvent : public muse::Event {
    // Declare friend to Event::create<> method can call protected constructor
    friend class muse::Event;
public:
    /** Convenience method to get any extra bytes associated with
        events.

        \note This method does not perform any special checks. So
        accessing data at the pointer returned by this method is
        contingent on the extra data associated with the event as
        returned by getExtraSize() method.
        
        \return A pointer to the extra bytes associated with events.
    */
    const char* getExtraData() const {
        // The padding buffer is at the end of this event. So simply
        // return pointer to end of the event.
        return reinterpret_cast<const char *>(this + 1);
    }

    /** \brief Get the size of this Event
        
        This method overrides the base class implementation to return
        the correct size of a PHOLDEvent.  The event size is important
        so that it can be properly sent across the wire by MPI.

        \return The size of the PHOLD event including any extra bytes.
    */
    virtual int getEventSize() const override { return totalEventSize; }
    
protected:
    /** The constructor for PHOLDEvent.

        The constructor is intentionally protected.  Don't use the
        constructor directly.  Instead use the Event::create
        templatized method to create events using MUSE's event
        recycling infrastructure:

        \code

        create(sizeof(PHOLDEvent) + extraBytes, receiverID, recvTime,
               extraBytes);

        \endcode

        \param[in] receiverID the id of the receiver agent
        
        \param[in] recvTime this is the time the receiving agent should
        process this event.

        \param[in] extraBytes Additional bytes to be added to the
        event.
    */
    PHOLDEvent(const muse::AgentID receiverID, const muse::Time recvTime,
               const int extraBytes) :
        muse::Event(receiverID, recvTime),
        totalEventSize(sizeof(PHOLDEvent) + extraBytes) {
        // Initialize extra data to all 1s if we have extra bytes
        if (extraBytes > 0) {
            char *data = reinterpret_cast<char*>(this + 1);
            for (int idx = 0; (idx < extraBytes); idx++) {
                data[idx] = 1;
            }
        }
    }
               
private:
    /** Instance variable to track the overall event size.  The extra
        bytes associated with the event is already added to the
        totalEventSize.  This enables providing a fast implementation
        for the getEventSize() virtual method which is frequently
        called, while compromising on the implementation of the
        getExtraSize() method in this class.  This value is set in the
        constructor and is never changed during the lifetime of this
        event.
    */
    int totalEventSize;
};

#endif
