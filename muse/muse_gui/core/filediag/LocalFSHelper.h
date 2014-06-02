#ifndef LOCAL_FS_HELPER_H
#define LOCAL_FS_HELPER_H

#include "FSHelperCommon.h"
#include "LocalFSHelper.h"
#include <QFileInfo>

class LocalFSHelper : public FSHelperCommon {
public:
    LocalFSHelper();
    ~LocalFSHelper();

    const FSEntryList* getEntries(const FSEntry& dir) const;
    // const FSEntryList* getEntries(const QString& dir) const;
    const FSEntry&     getRoot() const;

    QString  getHomePath()  const;
    QString  getSeparator() const;
    int      getColumns()   const { return 3; }

private:
    bool populateCache(const FSEntry& parentDir) const;
    FSEntry convert(const QFileInfo& info, const bool isDrive,
                    const FSEntry *parent) const;
};

#endif // LOCAL_FS_HELPER_H
