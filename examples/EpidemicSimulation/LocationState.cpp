
#include "LocationState.h"

LocationState::LocationState() : index(0) {}

muse::State*
LocationState::getClone(){
    //Keep in mind this shallow copy works because there are no complex
    //pointers that I need to take care of. Just primitive data types.
    LocationState* clone = new LocationState();
    clone->index = this->index;
    clone->timestamp = this->timestamp;
    
    return clone;
    //Caller should handle deleting the memory when done with State pointer
}
