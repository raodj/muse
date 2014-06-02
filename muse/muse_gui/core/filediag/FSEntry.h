#ifndef FSENTRY_H
#define FSENTRY_H

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

    inline FSEntry(const QString& fullPath, const long size = -1,
                   const long timestamp = -1, const int flags = 0,
                   const FSEntry *parent = NULL) :
        path(fullPath), size(size), timestamp(timestamp), flags(flags),
        parent(parent) {
        // Convenience constructor to create objects quickly.
    }

    ~FSEntry() {}

    inline       QString& getPath()       { return path; }
    inline const QString& getPath() const { return path; }


    inline long getSize()  const { return size; }
    inline long getTimestamp() const { return timestamp; }
    inline int  getFlags()     const { return flags; }
    inline const FSEntry* getParent() const { return parent; }

    inline bool operator<(const FSEntry& other) const {
        return (path > other.path);
    }

    inline bool operator==(const FSEntry& other) const {
        return path == other.path;
    }

    inline bool isValid() const { return (flags & INVALID_FLAG) == 0; }
    inline bool isDir() const { return (flags & DIR_FLAG) > 0; }
    inline bool isComputer() const { return (flags & COMPUTER_FLAG) > 0; }
    inline bool isDrive() const { return (flags & DRIVE_FLAG) > 0; }
    inline bool isLink() const { return (flags & LINK_FLAG) > 0; }
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
     * @return
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
