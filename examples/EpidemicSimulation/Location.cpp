#ifndef LOCATION_CPP
#define LOCATION_CPP

#include "Location.h"
#include "MTRandom.h"
#include "Simulation.h"
#include <cstdlib>
#include <cstdio>

Location::Location(const muse::AgentID id) : muse::Agent(id, new muse::State()) {
    // Nothing else for now.
}

#endif 
