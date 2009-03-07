/* 
  This is a template to quickly get going with development
  The usuless task of setting up the bases classes is done for you.
  Just change names around and get to the good part.
 */

#ifndef _SPACE_H
#define	_SPACE_H

#include "Agent.h"
#include "State.h"
using namespace muse;

class Space : public Agent {

public:
    Space(AgentID , State *);

    void initialize() throw (std::exception);

    void executeTask(const EventContainer* events);

    void finalize();

};

#endif	/* _SPACE_H */

