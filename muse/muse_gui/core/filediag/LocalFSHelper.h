#ifndef LOCAL_FS_HELPER_H
#define LOCAL_FS_HELPER_H

#include "FSHelperCommon.h"
#include "LocalFSHelper.h"
#include <QFileInfo>

/**
 * @brief The LocalFSHelper class A helper class that deals
 * specifically with local file system to perform operations for
 * the CustomFileDialog class on the local machine.
 */
class LocalFSHelper : public FSHelperCommon {
public:
    /**
     * @brief LocalFSHelper Constructor for the LocalFSHelper
     * class, which has no implementation at this time.
     */
    LocalFSHelper();
    ~LocalFSHelper();

    /**
     * @brief getEntries Gets a listing of entries at the specified
     * directory on the local file system.
     * @param dir The directory to get the listing of entries from.
     * @return The listing of entries.
     */
    const FSEntryList* getEntries(const FSEntry& dir) const;
    // const FSEntryList* getEntries(const QString& dir) const;

    /**
     * @brief getRoot Gets the root of the local file system,
     * which in most, if not all cases, is the hard drive.
     * @return The FSEntry of the root directory on the local
     * file system.
     */
    const FSEntry&     getRoot() const;

    /**
     * @brief getHomePath Gets the path to the home directory on
     * this local file system.
     * @return A QString representing the path to the home directory
     */
    QString  getHomePath()  const;

    /**
     * @brief getSeparator Gets the character used as a path separator
     * for the file system. In Qt, this is always "/", regardless of
     * the user's operating system.
     * @return The QString representation of the path separator.
     */
    QString  getSeparator() const;

    /**
     * @brief getColumns Gets the number of columns to use for
     * display in the CustomFileDialog.
     * @return The number of columns to use for displaying file and
     * directory information.
     */
    int      getColumns()   const { return 3; }

private:
    /**
     * @brief populateCache Populates the cache if it does not already
     * have the FSEntry given as a parameter in the cache. The method
     * informs the caller of the method if the cache has been populated.
     * @param parentDir The FSEntry to populate the cache with.
     * @return A boolean indicating if parentDir is in the cache after
     * calling this method.
     */
    bool populateCache(const FSEntry& parentDir) const;

    /**
     * @brief convert Converts a standard file/directory into an
     * FSEntry for use within the CustomFileDialog and its underlying
     * system.
     * @param info The QFileInfo about the file/directory to be converted.
     * @param isDrive If the boolean is a drive (true), or not (false).
     * @param parent A FSEntry pointer to this file/directory's parent.
     * @return An FSEntry for the converted file/directory.
     */
    FSEntry convert(const QFileInfo& info, const bool isDrive,
                    const FSEntry *parent) const;
};

#endif // LOCAL_FS_HELPER_H
