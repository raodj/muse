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

#ifndef _WINDOWS
#include <ext/hash_map>
#else
#include <hash_map>
#endif

#include <string.h>
#include "DataTypes.h"

#ifndef _WINDOWS
/** With GCC 3.2 and above the hash map data structure has been moded
    to a extended directory under a non-std namespace.  The following
    typedef attempts to put the hash map in the standard context
    making it much easier to work with.
*/
#define HashMap __gnu_cxx::hash_map
#define Hash    __gnu_cxx::hash

struct EqualStr {
    bool operator()(const char* s1, const char* s2) const {
        return strcmp(s1, s2) == 0;
    }
};

// Hasher for std::string. This hash function is needed to use std::string
// objects as key value in hash_map
struct StringHasher  {
    inline size_t operator()(const std::string& s) const {
        Hash<const char*> hasher;
        return hasher(s.c_str());
    }
};

/** \typedef A hash_map<const char *, int>

    A typedef for a hash map whose key is char* and contains integers.

    The following typedef provides a short cut for using a hash map 
    whose key is a C string and contains integers.
*/
typedef HashMap<const char *, int, Hash<const char *>, EqualStr> StringIntMap;


//Below is MUSE specific hash maps

//forward delcare so everyone is happy and fast.
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



#else  // Below is windows define

// In windows the following mappings are used.
#define HashMap stdext::hash_map

/** String comparison structure for const char *.

    The following structure essentially provides the comparison
    operator needed by the hash_map for comparing hash_key values.
    This structure specifically provides comparison for hash_map's
    whose key values are C strings.
*/
struct LessString {
    inline bool operator()(const char *s1, const char *s2) const {
        return strcmp(s1, s2) > 0;
    }
};

/** \typedef A hash_map<const char *, int>

    A typedef for a hash map whose key is char* and contains integers.

    The following typedef provides a short cut for using a hash map 
    whose key is a C string and contains integers.
*/
typedef HashMap<const char *, int, stdext::hash_compare<const char *, LessString> > StringIntMap;

#endif

/** Integer comparison structure for std::hash_map.

    The following structure essentially provides the comparison
    operator needed by the hash_map for comparing hash_key values.
    This structure specifically provides comparison for hash_map's
    whose key values are standard integers.
*/
struct EqualInteger {
    inline bool operator()(const int i1, const int i2) const {
        return (i1 == i2);
    }
};

//hash_map<Key, Data, HashFcn, EqualKey, Alloc>

#endif
