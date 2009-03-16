#ifndef SIMSTREAM_H
#define	SIMSTREAM_H

#include "DataTypes.h"

BEGIN_NAMESPACE(muse);

class SimStream {
public:
    virtual void saveState(const Time& lvt) = 0;
    virtual void rollback(const Time& restored_time) = 0;
    virtual void garbageCollect(const Time& gvt) = 0;
    
    virtual ~SimStream();

protected:
    SimStream();
};

END_NAMESPACE(muse);

#endif	/* SIMSTREAM_H */

