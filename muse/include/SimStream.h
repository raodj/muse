#ifndef _SIMSTREAM_H
#define	_SIMSTREAM_H

#include "DataTypes.h"

BEGIN_NAMESPACE(muse);

class SimStream {
public:
    virtual void saveState(const Time& lvt);
    virtual void rollback(const Time& restored_time);
    virtual void garbageCollect(const Time& gvt);
    virtual ~SimStream();
};

END_NAMESPACE(muse);
#endif	/* _SIMSTREAM_H */

