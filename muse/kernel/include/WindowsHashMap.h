#ifndef WINDOWS_HASH_MAP_H
#define WINDOWS_HASH_MAP_H

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
#error Do not include WindowsHashMap.h directly. Instead include HashMap.h
#endif

// In windows the following mappings are used.
#define HashMap stdext::hash_map
#define Hash    stdext::hash

/** String comparison structure for const char *.

    The following structure essentially provides the comparison
    operator needed by the hash_map for comparing hash_key values.
    This structure specifically provides comparison for hash_map's
    whose key values are C strings.
*/
struct LessString {
    inline bool operator()(const std::string& s1, const std::string& s2) const {
        return s1.compare(s2) < 0;
    }
};

/** \typedef A hash_map<std::string, int>

    A typedef for a hash map whose key is std::string and contains
    integers.
    
    The following typedef provides a short cut for using a hash map
    whose key is a std::string and contains integers.
*/
typedef HashMap<std::string, int, stdext::hash_compare<std::string, LessString> > StringIntMap;

/** \typedef A hash_map<std::string, std::string>

    A typedef for a hash map whose key is std::string and contains
    std::string objects.
    
    The following typedef provides a short cut for using a hash map
    whose key is a std::string and contains std::string objects..
*/
typedef HashMap<std::string, std::string, stdext::hash_compare<std::string, LessString> > StringStringMap;

#endif
