#ifndef UpdatePositionEvent_H
#define UpdatePositionEvent_H

#include "RescueEvent.h"
#include "VolunteerDataTypes.h"
using namespace muse;

class UpdatePositionEvent: public RescueEvent {
public:
   static UpdatePositionEvent* create(const muse::AgentID receiverID,
                                      const muse::Time recvTime, coord loc, VolunteerEventType type) {
      UpdatePositionEvent* event = reinterpret_cast<UpdatePositionEvent*>(new char[sizeof(UpdatePositionEvent)]);
      new (event) UpdatePositionEvent(receiverID, recvTime, loc, type);
      return event;
   }
   inline int getEventSize() const { return sizeof(UpdatePositionEvent); }
   inline coord getCurrentLocation() { return cur_location; }
protected:
   UpdatePositionEvent(AgentID receiver_id, Time receive_time, coord loc, VolunteerEventType type);
   coord cur_location;
};

#endif /* UpdatePositionEvent_H */
