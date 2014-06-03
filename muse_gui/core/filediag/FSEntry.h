#ifndef FSENTRY_H
#define FSENTRY_H
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

#include <QObject>
#include <QDebug>

/**
 * @brief The FSEntry class provides a cross-platform consistent API for
 * managing drives, files, and directories.
 *
 * This class represents the core entries in a file system, including:
 * drives, directories, and files entries.  The class exposes an
 * API that is independent of the underlying operating system.  Consequently,
 * the LocalFSHelper and RemoteFSHelper can utilize the FSEntry class to
 * represent file system artifacts in a consistent manner for populating
 * the generic FileSystemModel (which in turn is used for display
 * and other operations by the CustomFileDialog).
 *
 */
class FSEntry {
    friend QDebug operator<<(QDebug dbg, const FSEntry& entry);
public:

    static const int DIR_FLAG;
    static const int LINK_FLAG;
    static const int COMPUTER_FLAG;
    static const int INVALID_FLAG;
    static const int DRIVE_FLAG;
    static const int TEMP_FLAG;

    enum PermCategory { OWNER_PERM, GROUP_PERM, OTHER_PERM };

    inline FSEntry() : path(""), size(-1), timestamp(-1), flags(INVALID_FLAG), parent(NULL) {}

    /**
     * @brief FSEntry A conveninece constructor to create an FSEntry quickly.
     * @param fullPath The full path to the FSEntry being constructed.
     * @param size The size of the file/directory that is being turned into an FSEntry.
     * @param timestamp The timestamp on the file/directory that is being turned
     * into an FSEntry.
     * @param flags The set of flags to be initally set for the FSEntry.
     * @param parent A pointer to the FSEntry of this FSEntry's parent.
     */
    inline FSEntry(const QString& fullPath, const long size = -1,
                   const long timestamp = -1, const int flags = 0,
                   const FSEntry *parent = NULL) :
        path(fullPath), size(size), timestamp(timestamp), flags(flags),
        parent(parent) {
        // Convenience constructor to create objects quickly.
    }

    ~FSEntry() {}

    /**
     * @brief getPath Gets the path of this FSEntry.
     * @return A QString representation of the path to this FSEntry.
     */
    inline       QString& getPath()       { return path; }

    /**
     * @brief getPath Gets the path of this FSEntry.
     * @return A QString representation of the path to this FSEntry.
     */
    inline const QString& getPath() const { return path; }

    /**
     * @brief getSize Gets the size of this FSEntry.
     * @return The size of this FSEntry.
     */
    inline long getSize()  const { return size; }

    /**
     * @brief getTimestamp Gets the timestamp associated with this
     * FSEntry.
     * @return A long representing the timestamp.
     */
    inline long getTimestamp() const { return timestamp; }

    /**
     * @brief getFlags Gets the flags associated with this FSEntry.
     * @return An int representation of the flags.
     */
    inline int  getFlags()     const { return flags; }

    /**
     * @brief getParent Gets the parent of this FSEntry.
     * @return A pointer to the FSEntry to the parent of this
     * FSEntry.
     */
    inline const FSEntry* getParent() const { return parent; }

    inline bool operator<(const FSEntry& other) const {
        return (path > other.path);
    }

    inline bool operator==(const FSEntry& other) const {
        return path == other.path;
    }

    /**
     * @brief isValid Returns true if this FSEntry is valid.
     * @return A boolean indicating if this FSEntry is valid (true),
     * or not (false).
     */
    inline bool isValid() const { return (flags & INVALID_FLAG) == 0; }

    /**
     * @brief isDir Returns true if this FSEntry is a directory.
     * @return A boolean indicating if this FSEntry is a directory (true),
     * or not (false).
     */
    inline bool isDir() const { return (flags & DIR_FLAG) > 0; }

    /**
     * @brief isComputer Returns true if this FSEntry is a computer.
     * @return A boolean indicating if this FSEntry is a computer (true),
     * or not (false).
     */
    inline bool isComputer() const { return (flags & COMPUTER_FLAG) > 0; }

    /**
     * @brief isDrive Returns true if this FSEntry is a drive.
     * @return A boolean indicating if this FSEntry is a drive (true),
     * or not (false).
     */
    inline bool isDrive() const { return (flags & DRIVE_FLAG) > 0; }

    /**
     * @brief isLink Returns true if this FSEntry is a link.
     * @return A boolean indicating if this FSEntry is a link (true),
     * or not (false).
     */
    inline bool isLink() const { return (flags & LINK_FLAG) > 0; }

    /**
     * @brief isTempEntry Returns true if this FSEntry is a temporary
     * entry.
     * @return A boolean indicating if this FSEntry is a temporary entry (true),
     * or not (false).
     */
    inline bool isTempEntry() const { return (flags & TEMP_FLAG) > 0; }

    /**
     * @brief setPerm Helps build the permission bits for an FSEntry.
     *
     * This static method provides a convenience API for setting/constructing
     * file permissions. Specifically, this method provides an API for
     * composing bits for a specific category. This method is used by
     * LocalFSHelper and RemoteFSHelper classes to normalize the file
     * permissions from different operating systems to the consistent API
     * explosed by the FSEntry class.
     *
     * @param category The permissions category. This is one of the
     * predefined values corresponding to user, group, or owner permission.
     *
     * @param currPerms The current permissions of this FSEntry. This value
     * is embodied in the new set of permissions returned by this method.
     * The default initial value is zero.
     *
     * @param canRead The new permission on whether or not the file
     * can be read, from the context of the permission category specified.
     *
     * @param canWrite The new permission of whether or not the file
     * can be written to, from the context of the permission category.
     *
     * @param canExecute The new permission of whether or not the
     * file can be executed, from the context of the permission category.
     *
     * @return This method returns an aggregate set of permission bits
     * based on the currPerms and the set of permissions specified for
     * the given permission category.
     */
    static int setPerm(const PermCategory category, int currPerms,
                       const bool canRead, const bool canWrite,
                       const bool canExecute);

    /**
     * @brief setAttributes Sets the attributes of this FSEntry.
     * @param currPerms The current permissions of this FSEntry.
     * @param isDir Whether or not this FSEntry is a directory.
     * @param isLink Whether or not this FSEntry is a link.
     * @param isDrive Whether or not this FSEntry is a drive.
     * @return An aggregate set of attribute bits based on the
     * currPerms and the set attributes specified for the given
     * attribute category.
     */
    static int setAttributes(int currPerms, const bool isDir,
                             const bool isLink, const bool isDrive);

private:
    QString path;
    long    size;
    long    timestamp;
    int     flags;
    const FSEntry *parent;
};

// The following declaration is neecessary to use FSEntry
// in signals and slots. In addition a call to QRegisterMetaType
// is needed to register the object with the mdata-object system.
Q_DECLARE_METATYPE(FSEntry)

#endif // FSENTRY_H
