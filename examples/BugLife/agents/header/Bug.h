/* 
  This is a template to quickly get going with development
  The usuless task of setting up the bases classes is done for you.
  Just change names around and get to the good part.
 */

#ifndef _BUG_H
#define	_BUG_H

#include "Agent.h"
#include "State.h"
#include "BugDataTypes.h"

using namespace muse;

class Bug : public Agent {

public:
    Bug(AgentID , State *,CoordAgentIDMap *, int c, int r);

    void initialize() throw (std::exception);

    void executeTask(const EventContainer* events);

    void finalize();

    CoordAgentIDMap coord_map;
    int cols,rows;
    coord my_location;
};

#endif	/* _BUG_H */

