#ifndef PLAYER_DATA_TYPES_H
#define PLAYER_DATA_TYPES_H

#include <utility>
#include "DataTypes.h"
#include <map>


/** The following are the different event types in the players in the rescue simulation
 */
enum VolunteerEventType {UpdatePositionVictim, UpdatePositionVolunteer, VolunteerReport, UpdateNearby};

/** This used to represent a coord in the space for the players
 */
typedef pair<int, int> coord;

/** \typedef A map<y-coord, AgentID>

    A typedef for a hash map whose key is the y-coord and contains AgentID.

    player agents will use this to get the space agent id for a given coord.
*/
typedef map<int, muse::AgentID> CoordAgentIDMap;

/** The mod operator % really is a remainder operation.
    However we need true mod operator for wrapping around the space. The following macro
    implements true mod.
 */
#define true_mod(num,mod)((mod+(num%mod))%mod)

#define VOL_SIGNAL_RANGE 10

#define AREA_COL_WIDTH 10

#endif
