
#ifndef RLBACKOFFPOLICY_H
#define RLBACKOFFPOLICY_H

#include "PollPolicy.h"

class RLBackoffPolicy : public PollPolicy {
public:
    RLBackoffPolicy();
    virtual ~RLBackoffPolicy();
    virtual bool shouldPoll() override;
    virtual void updatePolicy(int numEvents = 0) override;
    virtual void forcePoll() override;
protected:
    
        
};

#endif /* RLBACKOFFPOLICY_H */

