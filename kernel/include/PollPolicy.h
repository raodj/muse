
#ifndef POLLPOLICY_H
#define POLLPOLICY_H

class PollPolicy {
public:
    PollPolicy();
    virtual ~PollPolicy();
    virtual bool shouldPoll() = 0;
    virtual void updatePolicy(int numEvents = 0) = 0;
    virtual void forcePoll() = 0;
};

#endif /* POLLPOLICY_H */

