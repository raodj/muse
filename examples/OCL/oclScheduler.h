#ifndef MUSE_OCLSCHEDULER_H
#define MUSE_OCLSCHEDULER_H
#include "../../kernel/include/Scheduler.h"
#include "OCLAgent.h"
BEGIN_NAMESPACE(muse);

class OCLAgent;
class oclScheduler : public Scheduler {
    friend class OCLAgent;
    public:
        
    /** The current 'top' Agent to process its Events
        
        \return True if the chosen agent had events to process.
    */
        bool processNextAgentEvents(OCLAgent** OCLAgent);

};
END_NAMESPACE(muse);
#endif