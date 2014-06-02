#include "LocalFSHelper.h"
#include <QDir>
#include <QThread>
#include <QDateTime>

#include <unistd.h>

LocalFSHelper::LocalFSHelper() {
    // Nothing to be done for now.
}

LocalFSHelper::~LocalFSHelper() {
    // Nothing to be done for now.
}

const FSEntry&
LocalFSHelper::getRoot() const {
    static const FSEntry root("localhost", -1, -1, FSEntry::COMPUTER_FLAG);
    return root;
}

QString
LocalFSHelper::getSeparator() const {
    return "/";
}

QString
LocalFSHelper::getHomePath() const {
    return QDir::homePath();
}

const FSEntryList*
LocalFSHelper::getEntries(const FSEntry& dir) const {
    // If base class has entry in cache, we return it immedately
    const FSEntryList *entryList = FSHelperCommon::getEntries(dir);
    if (entryList != NULL) {
        // Found entry in cache. Nothing further to do.
        return entryList;
    }
    // Entry not found in cache. We have to load entries.
    populateCache(dir);
    return FSHelperCommon::getEntries(dir);
}

/*
const FSEntryList*
LocalFSHelper::getEntries(const QString& dir) const {
    // If base class has entry in cache, we return it immedately
    const FSEntryList *entryList = FSHelperCommon::getEntries(dir);
    if (entryList != NULL) {
        // Found entry in cache. Nothing further to do.
        return entryList;
    }
    // Entry not found in cache. We have to load entries.
    populateCache(dir, FSHelperCommon::InvalidEntry);
    return FSHelperCommon::getEntries(dir);
}
*/

bool
LocalFSHelper::populateCache(const FSEntry &parentDir) const {
    Q_ASSERT (parentDir.isValid());
    // If the cache already contains an entry do nothing further.
    if (parentDir.isValid() && fsCache.contains(parentDir)) {
        return true;
    }
    // If the parent is not a directory, this is call is not very meanigful.
    const QDir dir(parentDir.getPath());
    if (!parentDir.isComputer() && !dir.exists()) {
        return false;
    }
    // Obtain corresponding entry list in the parent directory.
    const QFileInfoList dirEntries = parentDir.isComputer() ? QDir::drives() :
                dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    // Convert the entry information list into a portable FSEntry object
    FSEntryList cacheEntries;
    for(int i = 0; (i < dirEntries.size()); i++) {
        const QFileInfo& fileInfo = dirEntries.at(i);
        cacheEntries.append(convert(fileInfo, parentDir.isComputer(), &parentDir));
    }
    // Add entries to cache for future reference
    fsCache.insert(parentDir, cacheEntries);
    qDebug() << "Loaded data = " << parentDir << "(" << &parentDir << ")";
    return true;
}

FSEntry
LocalFSHelper::convert(const QFileInfo &info, const bool isDrive,
                       const FSEntry *parent) const {
    const int flags = FSEntry::setAttributes(0, info.isDir(),
                                             info.isSymLink(), isDrive) |
            FSEntry::setPerm(FSEntry::OWNER_PERM, 0, info.isReadable(),
                             info.isWritable(), info.isExecutable());
    return FSEntry(info.absoluteFilePath(), info.size(),
                   info.lastModified().toTime_t(), flags, parent);
}
