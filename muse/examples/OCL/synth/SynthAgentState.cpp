#ifndef OCL_SYNTH_AGENT_STATE_CPP
#define OCL_SYNTH_AGENT_STATE_CPP

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
// Authors: Dhananjai M. Rao       raodm@miamioh.edu
//
//---------------------------------------------------------------------------

#include <string>
#include <sstream>
#include "SynthAgentState.h"

// Initialize all 4 compartments wrapped inside hc sub-container to
// zeros.
SynthAgentState::SynthAgentState(const int compartments) :
    comps(compartments, 0) {
    // The agent will update compartments when the agent is
    // instantiated.
}

// The copy constructor initialize all instance variables from the src. 
SynthAgentState::SynthAgentState(const SynthAgentState& src) :
    comps(src.comps)  {
    // The copy constructor correctly initializes instance variables
    // and does not use assignment operator to assign value.
}

std::string
SynthAgentState::getHCStateDefinition() const {
    std::ostringstream os;
    os << "struct hc_state {\n"
       << "\treal comps[" << comps.size() << "];\n"
       << "};\n\n";
    return os.str();
}

#endif
