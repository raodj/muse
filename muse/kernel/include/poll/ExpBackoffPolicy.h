
#ifndef EXPBACKOFFPOLICY_H
#define EXPBACKOFFPOLICY_H

#include "poll/PollPolicy.h"

class ExpBackoffPolicy : public PollPolicy {
public:
    ExpBackoffPolicy();
    virtual ~ExpBackoffPolicy();
    virtual bool shouldPoll() override;
    virtual void updatePolicy(int numEvents = 0) override;
    virtual void forcePoll() override;
protected:
    int mpiMsgCheckCounter = 1;
    int mpiMsgCheckThresh = 1;
};

#endif /* EXPBACKOFFPOLICY_H */

