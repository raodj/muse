#ifndef FS_HELPER_COMMON_H
#define FS_HELPER_COMMON_H

#include "FSHelper.h"

class FSHelperCommon : public FSHelper {
public:
    /**
     * @brief getEntries Gets a listing of entries at the specified
     * directory.
     * @param dir The directory to get the listing of entries from.
     * @return The listing of entries.
     */
    virtual const FSEntryList* getEntries(const FSEntry& dir) const;

    /**
     * @brief getParent Gets the parent of a given FSEntry.
     * @param entry The entry whose parent is to be returned.
     * @return The FSEntry of the parent.
     */
    virtual const FSEntry&     getParent(const FSEntry& entry) const;

    /**
     * @brief getParentPath Gets the path of the parent of the given
     * directory/file.
     * @param path The path to the current directory/file.
     * @return The path to the parent as a QString.
     */
    virtual QString            getParentPath(const QString& path) const;
    // virtual const FSEntry&     getEntry(const QString& dir) const;

    /**
     * @brief getName Gets the name of the specified FSEntry.
     * @param entry The FSEntry whose name is needed.
     * @return The name of the FSEntry.
     */
    virtual QString getName(const FSEntry& entry) const;

    /**
     * @brief getName Gets the name of the item at the specified
     * file path.
     * @param entry The file path to the item.
     * @return The fileName of the item.
     */
    virtual QString getName(const QString& path) const;

    /**
     * @brief hasChildren Indicates whether or not the specified
     * directory has children.
     * @param dir The FSEntry of interest.
     * @return Whether or not the FSEntry has any children.
     */
    virtual bool    hasChildren(const FSEntry& dir) const;

    /**
     * @brief isCached Returns whether or not the given FSEntry is
     * cached.
     * @param dir The FSEntry to be checked for its cache status.
     * @return A boolean indicating the cached status of the given
     * FSEntry.
     */
    virtual bool isCached(const FSEntry& dir) const;

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
                                        const bool inCacheOnly = true);

    /**
     * @brief flushCaches Flushes the caches.
     */
    virtual void flushCaches();

    virtual ~FSHelperCommon();

protected:
    FSHelperCommon();
    void addEntry(const FSEntry& dir, const FSEntryList& entryList) const;

    /**
     * @brief getParentPath Gets the path of the parent of the
     * given FSEntry.
     * @param entry The FSEntry of the file/directory whose parent's
     * path is requested.
     * @return The path to the parent as a QString.
     */
    virtual QString getParentPath(const FSEntry& entry) const;

protected:
    mutable FSListMap fsCache;
    static const FSEntry InvalidEntry;
};

#endif // FS_HELPER_COMMON_H
