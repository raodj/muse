#include "FSHelperCommon.h"

const FSEntry FSHelperCommon::InvalidEntry;

FSHelperCommon::FSHelperCommon() {
    // Nothing to be done here for now.
}

FSHelperCommon::~FSHelperCommon() {
    // Nothing to be done here for now.
}

const FSEntryList*
FSHelperCommon::getEntries(const FSEntry& dir) const {
    FSListMap::const_iterator entry = fsCache.find(dir);
    if (entry != fsCache.end()) {
        // Found entry in the cache
        return &entry.value();
    }
    return NULL;
}

QString
FSHelperCommon::getParentPath(const FSEntry &entry) const {
    // Get the separator for this file system
    const QString sep = getSeparator();
    // An edge case for "/" or "C:/"
    if (getName(entry) == entry.getPath()) {
        return "";
    }
    // Identify index location of last separator occurrence in entry's path
    const int lastPos = std::max(1, entry.getPath().lastIndexOf(sep));
    // There should be at least one separator in the path at the end
    const bool haveSeparator = entry.getPath().indexOf(sep) < lastPos;
    // Return everything before lastPos as the parent path
    QString parentPath = entry.getPath().mid(0, lastPos + (haveSeparator ? 0 : 1));
    return parentPath;
}

QString
FSHelperCommon::getParentPath(const QString &path) const {
    // Get the separator for this file system
    const QString sep = getSeparator();
    // An edge case for "/" or "C:/" that have only one separator.
    if (getName(path) == path) {
        return "";
    }
    // Identify index location of last separator occurrence in entry's path
    const int lastPos = std::max(1, path.lastIndexOf(sep));
    // There should be at least one separator in the path at the end
    // (to handle cases where directory is of the form /home)
    const bool haveSeparator = path.indexOf(sep) < lastPos;
    // Return everything before lastPos as the parent path
    QString parentPath = path.mid(0, lastPos + (haveSeparator ? 0 : 1));
    return parentPath;
}

const FSEntry&
FSHelperCommon::getParent(const FSEntry& entry) const {
    if (!entry.isValid() || entry.isComputer() || (entry.getParent() == NULL)) {
        // Invalid entry or root entry. No parent for this one.
        return InvalidEntry;
    }
    // Now extract the parent path from the entry and return
    // it back to the caller after checking to ensure it is
    // present in the cache.
    const FSEntry& parent = *entry.getParent();
    Q_ASSERT( fsCache.contains(parent) );
    return parent;
}

bool
FSHelperCommon::hasChildren(const FSEntry& dir) const {
    FSListMap::const_iterator entry = fsCache.find(dir);
    if (entry != fsCache.end()) {
        // Found entry in the cache. Return actual result
        return entry.value().size() > 0;
    }
    // No cached entries. But don't bother enumerating files
    // in directories as the user may not drill-in.
    return (dir.isDir() || dir.isComputer() || dir.isDrive());
}

QString
FSHelperCommon::getName(const FSEntry &entry) const {
    return getName(entry.getPath());
}

QString
FSHelperCommon::getName(const QString& path) const {
    // Get the separator for this file system
    const QString sep = getSeparator();
    // Identify index location of last separator occurrence in entry's path
    const int lastPos = path.lastIndexOf(sep);
    // If separator is the last charachter then this must be
    // a drive or root of file system. So return the entry itself
    if (lastPos == path.size() - 1) {
        return path;
    }
    // Return everything before lastPos as the parent path
    return path.mid(lastPos + 1);

}

bool
FSHelperCommon::isCached(const FSEntry& dir) const {
    return fsCache.contains(dir);
}

void
FSHelperCommon::flushCaches() {
    fsCache.clear();
}

const FSEntryList*
FSHelperCommon::addEntry(const FSEntry& parent, const FSEntry& child,
                         const bool inCacheOnly) {
    Q_ASSERT(inCacheOnly == true);
    FSEntryList& entryList = fsCache[parent];
    entryList.append(child);
    return &entryList;
}

