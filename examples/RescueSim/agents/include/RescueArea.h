#ifndef RESCUE_AREA_H
#define RESCUE_AREA_H

#include "Agent.h"
#include "State.h"
#include "VolunteerDataTypes.h"

class RescueArea : public muse::Agent {
public:
    RescueArea(muse::AgentID, muse::State *);
    void initialize() throw (std::exception);
    void executeTask(const muse::EventContainer*);
    void finalize();

protected:
    /** Convenience method to determine number of entries to be copied
	into an event.

	This method is a simple convenience method that is used to
	determine the number of items to be copied into an event.
	Events have fixed number of items they can contain indicated
	by MAX_EVENT_ARRAY_SIZE constant.  Consequently, items that
	exceed this count are dispatched using multiple events.  This
	method is a convenience method to determine items to be packed
	into an event.

	\param[in] itemCount The total number of times to be packed
	into one or more event(s).

	\param[in] startPos The starting position (in the list of
	items) from where the data is to be copied.  If this value is
	greater then itemCount then this method returns zero.

	\param[in] maxItems An optional argument that indicates that
	the number of items that can be packaged into an event.
    */
    int eventItemCount(const size_t itemCount, int startPos,
		       const size_t maxItems = MAX_EVENT_ARRAY_SIZE) const;

private:
    // Helper method to dispatch nearby event
    void scheduleNearbyEvent(const muse::AgentID,
                             const std::vector<muse::AgentID>&,
                             const std::vector<coord>&,
                             const bool);
};

#endif
