
#ifndef AVGBACKOFFPOLICY_H
#define AVGBACKOFFPOLICY_H

#include "poll/PollPolicy.h"
#include "Avg.h"

class AvgBackoffPolicy : public PollPolicy {
public:
    AvgBackoffPolicy();
    virtual ~AvgBackoffPolicy();
    virtual bool shouldPoll() override;
    virtual void updatePolicy(int numEvents = 0) override;
    virtual void forcePoll() override;
protected:
    int mpiMsgCheckCounter = 1;
    int mpiMsgCheckThresh = 1;
    Avg maxMpiMsgCheckThresh;
};

#endif /* AVGBACKOFFPOLICY_H */

