#ifndef LINUX_HASH_MAP_H
#define LINUX_HASH_MAP_H

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
//
//---------------------------------------------------------------------------

#ifndef HASH_MAP_H
#error Do not include LinuxHashMap.h directly. Instead include HashMap.h
#endif

// The following compile guard is needed so that the hash map can be
// used for a test when running configure (because when configure is
// running, there is no config.h)
#ifdef REGULAR_COMPILE
#include "config.h"
#endif

// Use hash maps giving preference to the most recent version
#ifdef HAVE_UNORDERED_MAP
// Have the most modern std::unordered_map. Nice!
#include <unordered_map>
#define GLIBC_NAMESPACE std
#define HashMap std::unordered_map
#define Hash    std::hash

#elif defined(HAVE_TR1_UNORDERED_MAP)
// We must be compiling with gcc 4.2+ in which hash map has been moved
// to unordered_map in preperation for the Cx0 standard compliance. So
// use the latest and greatest standard
#include <tr1/unordered_map>
#define GLIBC_NAMESPACE std::tr1
#define HashMap std::tr1::unordered_map
#define Hash    std::tr1::hash

#elif defined(HAVE_EXT_HASH_MAP)
// Use standard hash map from the extended name space.  With GCC 3.2
// and above the hash map data structure has been moded to a extended
// directory under a non-std namespace.  The following defines
// attempts to put the hash map in the standard context making it much
// easier to work with.
#include <ext/hash_map>
#define GLIBC_NAMESPACE __gnu_cxx
#define HashMap __gnu_cxx::hash_map
#define Hash    __gnu_cxx::hash

#else // Default catch-all case. Typically never hit
// No TR1 and no EXT. Hopefully HAS_HASH_MAP is true
#include <hash_map>
#define GLIBC_NAMESPACE std
#define HashMap std::hash_map
#define Hash    std::hash

#endif

// Hasher for std::string. This hash function is needed to use std::string
// objects as key value in hash_map only in older version of compilers.
// Check and provide a backwards compatible hasher if needed.
#ifdef NEED_STRING_HASHER
// A slightly different definition is needed for ICC and gcc
#ifdef ICC
namespace __gnu_cxx {
    template<> struct Hash<std::string> {
        inline size_t operator()(const std::string& str) const {
            return Hash<const char*>()(str.c_str());
        }
    };
}
#else // compiler is not icc
struct StringHasher  {
    inline size_t operator()(const std::string& s) const {
        Hash<const char*> hasher;
        return hasher(s.c_str());

    }
};
#define USE_STRING_HASHER
#endif // icc vs. gcc differences

#else
// Use default system-provided string hasher
#define StringHasher Hash<std::string>
#endif

/** \typedef A hash_map<std::string, int>

    A typedef for a hash map whose key is std::string and contains integers.

    The following typedef provides a short cut for using a hash map 
    whose key is a C++ string and contains integers.
*/
#ifdef USE_STRING_HASHER
typedef HashMap<std::string, int, StringHasher> StringIntMap;
#else
typedef HashMap<std::string, int> StringIntMap;
#endif


/** \typedef A hash_map<std::string, std::string>

    A typedef for a hash map whose key is std::string and contains
    std::string objects.
    
    The following typedef provides a short cut for using a hash map
    whose key is a std::string and contains std::string objects..
*/
#ifdef USE_STRING_HASHER
typedef HashMap<std::string, std::string, StringHasher> StringStringMap;
#else
typedef HashMap<std::string, std::string> StringStringMap;
#endif
#endif // LINUX_HASH_MAP_H
