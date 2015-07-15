#ifndef REMOTEFSHELPER_CPP
#define REMOTEFSHELPER_CPP

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

#include "RemoteFSHelper.h"
#include "SshSocket.h"

#include <QThread>

RemoteFSHelper::RemoteFSHelper(RemoteServerSession* session, const bool deleteSocket) :
    /// THESE SHOULD NOT BE NULLPTR, MUST BE FIXED
    ssh(nullptr), deleteSocket(deleteSocket),
    root(nullptr, -1, -1, FSEntry::COMPUTER_FLAG) {
//    connect(this, SIGNAL(needSocket(QThread*,SshSocket*)),
//            this, SLOT(moveToThread(QThread*,SshSocket*)),
//            Qt::BlockingQueuedConnection);
//    if (session->getSftpChannel() == NULL) {
//        session->openSftpChannel();
//    }

//    sftp = session->getSftpChannel();
}

RemoteFSHelper::~RemoteFSHelper() {
    if (deleteSocket) {
        delete ssh;
    }
}

const FSEntry&
RemoteFSHelper::getRoot() const {
    return root;
}

QString
RemoteFSHelper::getSeparator() const {
    return "/";
}

void
RemoteFSHelper::moveToThread(QThread *thread, SshSocket *ssh) {
    ssh->changeToThread(thread);
    qDebug() << "Moved socket to thread.";
}

void
RemoteFSHelper::moveToThread(QThread* thread) {
    moveToThread(thread, ssh);
}

QString
RemoteFSHelper::getHomePath() const {
    QString home = "";
    try {
        // Create a Sftp channel to obtain home path.
        Q_ASSERT(ssh != NULL);
        home = sftp->getPwd();
    } catch (const SshException & se) {
        // How to log-it?
    }
    qDebug() << "* Home = " << home;
    return home;
}

const FSEntryList*
RemoteFSHelper::getEntries(const FSEntry& dir) const {
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

bool
RemoteFSHelper::populateCache(const FSEntry &parentDir) const {
    Q_ASSERT (parentDir.isValid());
    // If the cache already contains an entry do nothing further.
    if (parentDir.isValid() && fsCache.contains(parentDir)) {
        return true;
    }
    // Get path based on the parent directory
    QString path = parentDir.isComputer() ? "/" : parentDir.getPath();
    path += (path.endsWith("/") ? "" : "/");
    // The following list is populated in the following try-catch block.
    FSEntryList cacheEntries;
    try {
        // Create a Sftp channel to obtain the necessary information
        Q_ASSERT(ssh != NULL);
        if (ssh->thread() != QThread::currentThread()) {
            emit const_cast<RemoteFSHelper*>(this)->needSocket(QThread::currentThread(), ssh);
        }

        // Get the convenience class to obtain entries in the directory
        SFtpDir dir = sftp->getDir(path);
        // Depending on parent add the root directory or entries in the directory
        parentDir.isComputer() ? addDir(parentDir, dir, cacheEntries) :
                                 addEntries(parentDir, dir, cacheEntries);
        // Add the cache entries to the cache.
        fsCache.insert(parentDir, cacheEntries);
        qDebug() << "Loaded data = " << parentDir << "(" << &parentDir << ")";
        return true;
    } catch (const SshException &se) {
        // Should we log it somehow?
        qDebug() << se.getMessage();
        // Add an empty cache entry to avoid repeated checking.
        fsCache.insert(parentDir, cacheEntries);
    }
    // Failed to add entries to cache!
    return false;
}

void
RemoteFSHelper::addDir(const FSEntry &parentDir,
                       SFtpDir &dir, FSEntryList &list) const {
    // Get information about the directory itself.
    long size  = -1, timestamp = -1;
    int  flags = -1;
    dir.getInfo(size, timestamp, flags);
    // Add information to the list.
    list.append(FSEntry(dir.getPath(), size, timestamp,
                        convert(flags), &parentDir));

}

void
RemoteFSHelper::addEntries(const FSEntry &parentDir,
                           SFtpDir &dir, FSEntryList &list) const {
    // Extract and setup path for use further below.
    const QString path = dir.getPath();
    // Repeatedly get entry and add it to the cacheEntries list
    QString name;
    long size  = -1, timestamp = -1;
    int  flags = -1;
    while (dir.getEntry(name, size, timestamp, flags)) {
        if ((name == ".") || (name == "..")) {
            continue;
        }
        // Convert the entry information list into a portable FSEntry object
        list.append(FSEntry(path + name, size, timestamp,
                            convert(flags), &parentDir));
    }
}

int
RemoteFSHelper::convert(const int sftpFlags) const {
    const int flags =
            FSEntry::setAttributes(0, sftpFlags & LIBSSH2_SFTP_S_IFDIR,
                                   sftpFlags & LIBSSH2_SFTP_S_IFLNK,
                                   0) |
            FSEntry::setPerm(FSEntry::OWNER_PERM, 0,
                             sftpFlags & LIBSSH2_SFTP_S_IRUSR,
                             sftpFlags & LIBSSH2_SFTP_S_IWUSR,
                             sftpFlags & LIBSSH2_SFTP_S_IXUSR);
    return flags;
}

#endif
