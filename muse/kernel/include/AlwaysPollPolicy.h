
#ifndef ALWAYSPOLLPOLICY_H
#define ALWAYSPOLLPOLICY_H

#include "PollPolicy.h"

class AlwaysPollPolicy : public PollPolicy {
public:
    AlwaysPollPolicy();
    virtual ~AlwaysPollPolicy();
    virtual bool shouldPoll() override;;
    virtual void updatePolicy(int numEvents = 0) override;;
    virtual void forcePoll() override;
};

#endif /* ALWAYSPOLLPOLICY */

