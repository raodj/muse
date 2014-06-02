#ifndef FSENTRY_H
#define FSENTRY_H

#include <QObject>
#include <QDebug>

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

    static int setPerm(const PermCategory category, int currPerms,
                       const bool canRead, const bool canWrite,
                       const bool canExecute);

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
