#ifndef LOCAL_FS_HELPER_CPP
#define LOCAL_FS_HELPER_CPP
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

#include "LocalFSHelper.h"
#include <QDir>
#include <QThread>
#include <QDateTime>

#include <unistd.h>

LocalFSHelper::LocalFSHelper() {
    // Nothing to be done for now.
}

LocalFSHelper::~LocalFSHelper() {
    // Nothing to be done for now.
}

const FSEntry&
LocalFSHelper::getRoot() const {
    static const FSEntry root("localhost", -1, -1, FSEntry::COMPUTER_FLAG);
    return root;
}

QString
LocalFSHelper::getSeparator() const {
    return "/";
}

void
LocalFSHelper::moveToThread(QThread* thread) {
    Q_UNUSED(thread);
}

QString
LocalFSHelper::getHomePath() const {
    return QDir::homePath();
}

const FSEntryList*
LocalFSHelper::getEntries(const FSEntry& dir) const {
    // If base class has entry in cache, we return it immedately
    const FSEntryList *entryList = FSHelperCommon::getEntries(dir);
    if (entryList != NULL) {
        // Found entry in cache. Nothing further to do.
        return entryList;
    }
    // Entry not found in cache. We have to load entries.
    populateCache(dir);
    return FSHelperCommon::getEntries(dir);
}

/*
const FSEntryList*
LocalFSHelper::getEntries(const QString& dir) const {
    // If base class has entry in cache, we return it immedately
    const FSEntryList *entryList = FSHelperCommon::getEntries(dir);
    if (entryList != NULL) {
        // Found entry in cache. Nothing further to do.
        return entryList;
    }
    // Entry not found in cache. We have to load entries.
    populateCache(dir, FSHelperCommon::InvalidEntry);
    return FSHelperCommon::getEntries(dir);
}
*/

bool
LocalFSHelper::populateCache(const FSEntry &parentDir) const {
    Q_ASSERT (parentDir.isValid());
    // If the cache already contains an entry do nothing further.
    if (parentDir.isValid() && fsCache.contains(parentDir)) {
        return true;
    }
    // If the parent is not a directory, this is call is not very meanigful.
    const QDir dir(parentDir.getPath());
    if (!parentDir.isComputer() && !dir.exists()) {
        return false;
    }
    // Obtain corresponding entry list in the parent directory.
    const QFileInfoList dirEntries = parentDir.isComputer() ? QDir::drives() :
                dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    // Convert the entry information list into a portable FSEntry object
    FSEntryList cacheEntries;
    for(int i = 0; (i < dirEntries.size()); i++) {
        const QFileInfo& fileInfo = dirEntries.at(i);
        cacheEntries.append(convert(fileInfo, parentDir.isComputer(), &parentDir));
    }
    // Add entries to cache for future reference
    fsCache.insert(parentDir, cacheEntries);
    qDebug() << "Loaded data = " << parentDir << "(" << &parentDir << ")";
    return true;
}

FSEntry
LocalFSHelper::convert(const QFileInfo &info, const bool isDrive,
                       const FSEntry *parent) const {
    const int flags = FSEntry::setAttributes(0, info.isDir(),
                                             info.isSymLink(), isDrive) |
            FSEntry::setPerm(FSEntry::OWNER_PERM, 0, info.isReadable(),
                             info.isWritable(), info.isExecutable());
    return FSEntry(info.absoluteFilePath(), info.size(),
                   info.lastModified().toTime_t(), flags, parent);
}

#endif
