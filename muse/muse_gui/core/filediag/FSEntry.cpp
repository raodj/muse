#ifndef FS_ENTRY_CPP
#define FS_ENTRY_CPP
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
    // category and then sift it left depending on PermCategory
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

#endif
