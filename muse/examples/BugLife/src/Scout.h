#ifndef SCOUT_H
#define SCOUT_H

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

#include "BugEvent.h"

/* The scout event class to aid a Bug agent to scout spaces around it.
   
   This event is used for a bug to scout the spaces around. Things 
   of interest to the bug would include the food count and predator
   is in the space.

   \note This event should be created by calling Scout::create()
   method to ensure that memory is correctly allocated (to be flat) so
   that events can be safely and easily sent to other parallel kernels
   (possibly running on other compute nodes).
*/
class Scout : public BugEvent {
public:
    /** Interface method to create a Scout event.

	This method must be used to create a Scout event.  This method
	allocates a flat chunk of memory for the event (as per MUSE
	API requirements) to ensure that it is safe to send Scout
	events to agents on other parallel MUSE kernels (running on
	different compute nodes)

	\param[in] receiverID The target/destination agent to which
	the event should be sent.  This value is assumed to correspond
	to a Space agent (and no additional checks are performed).
	
	\param[in] receiveTime The time (in the future) when this event
	should be processed by the target space agent.
    */
    static Scout* create(const muse::AgentID& receiverID,
			 const muse::Time& receiveTime) {
	// Create flat memory for event
        Scout* event = reinterpret_cast<Scout*>(new char[sizeof(Scout)]);
	// Initialize flat memory as an object
        new (event) Scout(receiverID, receiveTime);
	// Return newly created event.
        return event;
    }
    
    /** Override MUSE base class implementation to report correct
	event size.

	This method must be overridden in all event classes to report
	correct size of the event (in bytes) to the MUSE kernel.

	\return The size of the Scout event (in bytes) as per MUSE API
	requirements.
    */
    inline int getEventSize() const { return sizeof(Scout); }

    /** the space sets this value to report how much food is in the
	space at the time of receving this event.
	
	@since Version 11
    */
    int foodCount;
    
protected:
    /** The constructor.

	The constructor has been made private to ensure it is never
	directly called.  Instead the Scout::create() method must be
	used to create an event of this type.

	\param[in] receiverID The target/destination agent to which
	the event should be sent.  This value is assumed to correspond
	to a Space agent (and no additional checks are performed).
	
	\param[in] receiveTime The time (in the future) when this event
	should be processed by the target space agent.	
     */
    Scout(const muse::AgentID& receiverID, const muse::Time& receiveTime);
};

#endif





