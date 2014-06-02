#ifndef FS_HELPER_COMMON_H
#define FS_HELPER_COMMON_H

#include "FSHelper.h"

class FSHelperCommon : public FSHelper {
public:
    virtual const FSEntryList* getEntries(const FSEntry& dir) const;
    virtual const FSEntry&     getParent(const FSEntry& entry) const;
    virtual QString            getParentPath(const QString& path) const;
    // virtual const FSEntry&     getEntry(const QString& dir) const;

    virtual QString getName(const FSEntry& entry) const;
    virtual QString getName(const QString& path) const;
    virtual bool    hasChildren(const FSEntry& dir) const;

    virtual bool isCached(const FSEntry& dir) const;

    virtual const FSEntryList* addEntry(const FSEntry& parent,
                                        const FSEntry& child,
                                        const bool inCacheOnly = true);

    virtual void flushCaches();

    virtual ~FSHelperCommon();

protected:
    FSHelperCommon();
    void addEntry(const FSEntry& dir, const FSEntryList& entryList) const;
    virtual QString getParentPath(const FSEntry& entry) const;

protected:
    mutable FSListMap fsCache;
    static const FSEntry InvalidEntry;
};

#endif // FS_HELPER_COMMON_H
