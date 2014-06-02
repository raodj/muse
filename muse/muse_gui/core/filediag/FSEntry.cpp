#include "FSEntry.h"
#include <QFile>

const int FSEntry::DIR_FLAG      = 0x100000;
const int FSEntry::LINK_FLAG     = 0x200000;
const int FSEntry::COMPUTER_FLAG = 0x400000;
const int FSEntry::INVALID_FLAG  = 0x800000;
const int FSEntry::DRIVE_FLAG    = 0x1000000;
const int FSEntry::TEMP_FLAG     = 0x2000000;

QDebug operator<<(QDebug dbg, const FSEntry& entry) {
    dbg.nospace() << entry.path << " (" << entry.size << ") "
                  << entry.timestamp
                  << ", " << entry.flags;
    return dbg.maybeSpace();
}

int
FSEntry::setPerm(const PermCategory category, int currPerms,
                 const bool canRead, const bool canWrite,
                 const bool canExecute) {
    // First we compse lower set of bits associated with "Other"
    // catoegry and then sift it left depending on PermCategory
    const int currFlags = (canRead ? QFile::ReadOther : 0) |
            (canWrite   ? QFile::WriteOther : 0) |
            (canExecute ? QFile::ExeOther   : 0);
    // Shift bits left (the switch is intentionally missing break;)
    switch (category) {
    case OWNER_PERM: (currFlags << 3);
    case GROUP_PERM: (currFlags << 3);
    case OTHER_PERM: ;
    }

    return (currPerms | currFlags);
}

int
FSEntry::setAttributes(int currPerms, const bool isDir,
                       const bool isLink, const bool isDrive) {
    return (currPerms | (isDir ? DIR_FLAG : 0) |
            (isLink ? LINK_FLAG : 0) | (isDrive ? DRIVE_FLAG : 0));
}
