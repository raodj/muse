#ifndef BUG_DATA_TYPES_H
#define BUG_DATA_TYPES_H

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
// Authors: Meseret Gebre       meseret.gebre@gmail.com
//
//---------------------------------------------------------------------------


#include <utility>
#include "HashMap.h"
#include "DataTypes.h"
/** This used to represent a coord in the space for the bugs
 */
typedef pair<int,int> coord;

/** coord comparison structure for std::hash_map.
    This structure provides comparison for hash_map's
    whose key values are coord.
*/
struct EqualCoord {
  inline size_t operator()(const coord c1,const coord c2) const {
        return (a1 == a2);
    }
};

/** \typedef A hash_map<coord, AgentID>

    A typedef for a hash map whose key is coord and contains AgentID.

    Bug agents will use this to get the space agent id for a given coord.
*/
typedef HashMap<coord, muse::AgentID, Hash<coord>, EqualCoord> CoordAgentIDMap;

#endif
