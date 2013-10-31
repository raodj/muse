#ifndef HASH_MAP_H
#define HASH_MAP_H

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OH.
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
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//          Meseret Gebre          gebremr@muohio.edu
//
//---------------------------------------------------------------------------

#include <cstring>

#ifndef _WINDOWS
#include "LinuxHashMap.h"
#else
#include "WindowsHashMap.h"
#endif

#include "DataTypes.h"

// Below are MUSE specific hash maps

// forward delcare so everyone is happy and fast.
namespace muse {
    class Agent;
}

/** AgentID comparison structure for std::hash_map.
    This structure provides comparison for hash_map's
    whose key values are AgentID.
*/
struct EqualAgentID {
    inline size_t operator()(const muse::AgentID a1,
                             const muse::AgentID a2) const {
        return (a1 == a2);
    }
};

/** \typedef A hash_map<AgentID, SimulatorID>

    A typedef for a hash map whose key is AgentID and contains SimulatorID.

    The following typedef provides a short cut for using a hash map
    whose key is a AgentID and contains SimulatorID.
*/
typedef HashMap<muse::AgentID, muse::SimulatorID, Hash<muse::AgentID>, EqualAgentID> AgentIDSimulatorIDMap;

/** \typedef A hash_map<AgentID, muse::Agent*>

    A typedef for a hash map whose key is AgentID and contains Agent*.

    The following typedef provides a short cut for using a hash map
    whose key is a AgentID and contains Agent*.
*/
typedef HashMap<muse::AgentID, muse::Agent*, Hash<muse::AgentID>, EqualAgentID> AgentIDAgentPointerMap;

/** \typedef A hash_map<AgentID, bool>

    A typedef for a hash map whose key is AgentID and contains booleans.

    The following typedef provides a short cut for using a hash map
    whose key is a AgentID and contains booleans.
*/
typedef HashMap<muse::AgentID, bool, Hash<muse::AgentID>, EqualAgentID> AgentIDBoolMap;

#endif  // HASH_MAP_H
