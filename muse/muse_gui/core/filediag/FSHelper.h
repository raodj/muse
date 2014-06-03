#ifndef FS_HELPER_H
#define FS_HELPER_H

#include "FSEntry.h"
#include <QList>

typedef QList<FSEntry> FSEntryList;
typedef QMap<FSEntry, FSEntryList> FSListMap;

class FSHelper : public QObject {
    Q_OBJECT
public:
    /**
     * @brief getEntries Gets a listing of entries at the specified
     * directory.
     * @param dir The directory to get the listing of entries from.
     * @return The listing of entries.
     */
    virtual const FSEntryList* getEntries(const FSEntry& dir) const = 0;

    /**
     * @brief getRoot Gets the root of the current file system.
     * @return The FSEntry of the root in the current file system.
     */
    virtual const FSEntry&     getRoot() const = 0;

    /**
     * @brief getParent Gets the parent of a given FSEntry.
     * @param entry The entry whose parent is to be returned.
     * @return The FSEntry of the parent.
     */
    virtual const FSEntry&     getParent(const FSEntry& entry) const = 0;

    /**
     * @brief getParentPath Gets the path of the parent of the given
     * directory/file.
     * @param path The path to the current directory/file.
     * @return The path to the parent as a QString.
     */
    virtual QString            getParentPath(const QString& path)  const = 0;

    /**
     * @brief getHomePath Gets the path to the home directory on
     * this local file system.
     * @return A QString representing the path to the home directory
     */
    virtual QString            getHomePath() const = 0;

    /**
     * @brief getSeparator Gets the character used as a path separator
     * for the file system. In Qt, this is always "/", regardless of
     * the operating system.
     * @return The QString representation of the path separator.
     */
    virtual QString getSeparator() const = 0;

    /**
     * @brief getName Gets the name of the specified FSEntry.
     * @param entry The FSEntry whose name is needed.
     * @return The name of the FSEntry.
     */
    virtual QString getName(const FSEntry& entry) const = 0;

    /**
     * @brief getName Gets the name of the item at the specified
     * file path.
     * @param entry The file path to the item.
     * @return The fileName of the item.
     */
    virtual QString getName(const QString& entry) const = 0;

    /**
     * @brief getColumns Gets the number of columns to use for
     * display in the CustomFileDialog.
     * @return The number of columns to use for displaying file and
     * directory information.
     */
    virtual int     getColumns() const = 0;

    /**
     * @brief hasChildren Indicates whether or not the specified
     * directory has children.
     * @param dir The FSEntry of interest.
     * @return Whether or not the FSEntry has any children.
     */
    virtual bool    hasChildren(const FSEntry& dir) const = 0;

    /**
     * @brief isCached Returns whether or not the given FSEntry is
     * cached.
     * @param dir The FSEntry to be checked for its cache status.
     * @return A boolean indicating the cached status of the given
     * FSEntry.
     */
    virtual bool    isCached(const FSEntry& dir) const = 0;

    /**
     * @brief flushCaches Flushes the caches.
     */
    virtual void    flushCaches() = 0;

    /**
     * @brief addEntry Adds an entry to the entry list.
     * @param parent The parent of the FSEntry to be added.
     * @param child The FSEntry to be added to the entry list.
     * @param inCacheOnly A boolean that indicates if the child is
     * only in the cache. This MUST be true for the method to succeed.
     * @return A pointer to the FSEntryList.
     */
    virtual const FSEntryList* addEntry(const FSEntry& parent,
                                        const FSEntry& child,
                                        const bool inCacheOnly = true) = 0;
    virtual ~FSHelper() {}

protected:
    FSHelper() {}
};

#endif // FS_HELPER_H
