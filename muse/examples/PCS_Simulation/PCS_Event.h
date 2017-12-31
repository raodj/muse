#ifndef PCS_EVENT_H
#define PCS_EVENT_H

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

/* 
 * File:   PCS_Event.h
 * Author: Julius Higiro
 *
 * Created on August 31, 2016, 10:09 AM
 */

#include "Event.h"

enum NextAction {
    NEXTCALL,
    MOVECALL,
    COMPLETECALL
};

enum Method {
    NEXT_CALL,
    MOVE_IN,
    MOVE_OUT,
    COMPLETE_CALL
};

/**
 * The PCS_Event class is used to create timestamped events that
 * represent a Portable in the PCS model.  The Portable represents a
 * mobile phone unit that resides within the Cell for a period of time
 * and then moves to one of the four neighboring Cells. The behavior
 * of a Portable (such as: move, call arrival, and call completion)
 * are modeled by different instance variables in this PCS_Event class
 * In similarity to a Portable, the PCS_Event object has three
 * independent exponentially distributed timestamp fields.
 *
 * <p>
 * This event class is instantiated as suggested below:
 *
 * \code
 * PCS_Event* const event =
 *     muse::Event::create<PCS_Event>(receiver, recvTime, moveTime,
 *                                    callTime, compTime, Method, portable);
 * \endcode
 *
 * </p>
 */
class PCS_Event : public muse::Event {
    // Decleare muse::Event as friend so it can call constructors here
    friend class muse::Event;
public:
    inline muse::Time getMoveTimeStamp() const {
        return moveTimeStamp;
    }
    
    inline muse::Time getNextCallTimeStamp() const {
        return nextCallTimeStamp;
    }
    
    inline muse::Time getCompletionTimeStamp() const {
        return completionTimeStamp;
    }
    
    inline Method getMethod() const {
        return method;
    }

    inline uint getPortableID() const {
        return portableID;
    }
    
    /** \brief Get the size of this Event
        
        This method is used to determine the correct size of each
        Event so that it can be properly sent across the wire by MPI.

        \note Users should override this method if event class is
        inherited and return correct size of the inherited event.

        \return The size of the event
    */
    virtual int getEventSize() const override { return sizeof(PCS_Event); }
    
protected:
    /** The move timestamp represents the time at which the portable
        will leave the current cell and re- side at one of the
        neighboring cells.  It is initialized by an exponentially
        distributed random number.
    */
    muse::Time moveTimeStamp;

    /** The next call timestamp represents the time when the next call
        will arrive for this portable.  It is initialized by an
        exponentially distributed random number.
    */
    muse::Time nextCallTimeStamp;

    /** The call completion timestamp represents the time at which
        this "portable" will complete the current call. If a call is
        not in progress the call completion timestamp is infinity. We
        assume that no calls are initially in progress.
        
    */
    muse::Time completionTimeStamp;

    /** This instance variable represents the current state of the
        portable -- i.e., whether it is in a call, moving between
        cells, or completing a call.  Whenever a portable changes its
        state a new event is created with a different method value and
        different timestamps.
    */
    Method method;

    /** A simple identifier associated with a portable.  This value is
        set the first time a portable is created and is consistently
        progpagated from event-to-event so that the state transitions
        of a portable can be tracked (if needed) across the simulation.
    */
    uint portableID;
    
    /** The constructor that merely initializes the instance variables
        to the corresponding parameter values.

        The constructor is invoked from the static create() method in
        this class.  The constructor is straightforward and merely
        initializes the instance variables to values specified in the
        parameter.
    */
    PCS_Event(muse::AgentID receiver_id, muse::Time receive_time,
              muse::Time moveTimeStamp, muse::Time nextCallTimeStamp,
              muse::Time completionTimeStamp, Method method,
              uint portableID);
};


#endif /* PCSEVENT_H */

