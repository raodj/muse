#ifndef LOCAL_FS_HELPER_H
#define LOCAL_FS_HELPER_H
//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

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

public slots:
    /**
     * @brief moveToThread This method is not relevant to this class,
     * and so this method has no implementation. It is needed, though
     * for OOP uses throught MUSE.
     * @param thread
     * @param ssh
     */
    void moveToThread(QThread *thread);

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
