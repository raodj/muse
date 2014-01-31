#ifndef BUG_EVENT_H
#define BUG_EVENT_H

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
#include "BugDataTypes.h"

/** The base class of all events in the BugLife simulation.

    This class serves as the abstract base class of all events in the
    BugLife simulation.  The base class primarily serves to contain
    the type of the event so that different types of muse::Event
    delivered to various agents in the BugLife simulation can be
    easily identified, safely typecasted, and suitably processed.

    \note This class is not meant to be directly instantiated. Instead
    create one of the derived events via suitable calls to the
    corresponding static create() methods associated with each event
    class.
*/
class BugEvent : public muse::Event {
public:
    /** Determine the type of this BugLife event.

	This is a convenience method that returns the type of this
	BugEvent.  This method provides a convenient mechanism to
	identify different types of events used in the simulation.
    */
    inline BugEventType getEventType()const { return event_type;}
    
protected:
    /**
        The type of BugLife simulation event. This value is set when a
        derived event class is instantiated and is never changed
        during the life time of an event.
    */
    const BugEventType event_type;

    /** The constructor.

	The constructor is protected to ensure that this class is
	never directly instantiated.  Instead one of the derived event
	classes should be instantiated (using suitable static create()
	method calls) and used.

	\param[in] receiverID The target/destination agent to which
	the event should be sent.  This value is assumed to correspond
	to a Space agent (and no additional checks are performed).
	
	\param[in] receiveTime The time (in the future) when this event
	should be processed by the target space agent.

	\param[in] e_type A predefined enumeration value identifying
	the type of BugEvent; that is the type of the derived event
	class.
    */
    BugEvent(const muse::AgentID& receiverID, const muse::Time& receiveTime,
	     const BugEventType e_type);
};

#endif

