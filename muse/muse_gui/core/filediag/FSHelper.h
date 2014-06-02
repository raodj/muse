#ifndef FS_HELPER_H
#define FS_HELPER_H

#include "FSEntry.h"
#include <QList>

typedef QList<FSEntry> FSEntryList;
typedef QMap<FSEntry, FSEntryList> FSListMap;

class FSHelper : public QObject {
    Q_OBJECT
public:
    virtual const FSEntryList* getEntries(const FSEntry& dir) const = 0;
    virtual const FSEntry&     getRoot() const = 0;
    virtual const FSEntry&     getParent(const FSEntry& entry) const = 0;
    virtual QString            getParentPath(const QString& path)  const = 0;
    virtual QString            getHomePath() const = 0;

    virtual QString getSeparator() const = 0;
    virtual QString getName(const FSEntry& entry) const = 0;
    virtual QString getName(const QString& entry) const = 0;
    virtual int     getColumns() const = 0;
    virtual bool    hasChildren(const FSEntry& dir) const = 0;
    virtual bool    isCached(const FSEntry& dir) const = 0;
    virtual void    flushCaches() = 0;

    virtual const FSEntryList* addEntry(const FSEntry& parent,
                                        const FSEntry& child,
                                        const bool inCacheOnly = true) = 0;
    virtual ~FSHelper() {}

protected:
    FSHelper() {}
};

#endif // FS_HELPER_H
