#ifndef REMOTEFSHELPER_H
#define REMOTEFSHELPER_H
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
#include "SFtpChannel.h"
#include "RemoteServerSession.h"

/**
 * @brief The RemoteFSHelper class A class that helps with file
 * operations on remote file systems. This class is part of the
 * CustomFileDialog system. This class will use the ssh2 library
 * for performing operations on the remote file system.
 */
class RemoteFSHelper : public FSHelperCommon {
    Q_OBJECT
public:
    /**
     * @brief RemoteFSHelper The constructor for the RemoteFSHelper
     * class.
     *
     * @param ssh Pointer to the ssh socket that has been established
     * with the remote computer.
     *
     * @param deleteSocket Whether or not the socket should be
     * deleted when this class is deleted.
     */
    RemoteFSHelper(RemoteServerSession* session, const bool deleteSocket = false);
    ~RemoteFSHelper();

    /**
     * @brief getEntries Gets a listing of entries at the specified
     * directory.
     *
     * @param dir The directory to get the listing of entries from.
     *
     * @return The listing of entries.
     */
    const FSEntryList* getEntries(const FSEntry& dir) const;

    /**
     * @brief getRoot Gets the root of the current file system.
     *
     * @return The FSEntry of the root in the current file system.
     */
    const FSEntry& getRoot() const;

    /**
     * @brief getHomePath Gets the path to the home directory on
     * this local file system.
     *
     * @return A QString representing the path to the home directory
     */
    QString getHomePath()  const;

    /**
     * @brief getSeparator Gets the character used as a path separator
     * for the file system. In Qt, this is always "/", regardless of
     * the operating system.
     *
     * @return The QString representation of the path separator.
     */
    QString getSeparator() const;

    /**
     * @brief getColumns Gets the number of columns to use for
     * display in the CustomFileDialog.
     *
     * @return The number of columns to use for displaying file and
     * directory information.
     */
    int getColumns() const { return 3; }

protected:
    /**
     * @brief populateCache Populates the cache if it does not already
     * have the FSEntry given as a parameter in the cache. The method
     * informs the caller of the method if the cache has been populated.
     * @param parentDir The FSEntry to populate the cache with.
     * @return A boolean indicating if parentDir is in the cache after
     * calling this method.
     */
    bool populateCache(const FSEntry& parentDir) const;
    void addEntries(const FSEntry& parentDir, SFtpDir& dir, FSEntryList& list) const;

    /**
     * @brief addDir Adds the given directory to the listing of FSEntries.
     * This method throws an SshException if an error occurs.
     * @param parentDir The FSEntry of the directory to add to the list.
     * @param dir The SFtpDir of the directory on the remote server.
     * @param list The list dir will be added to.
     */
    void addDir(const FSEntry &parentDir, SFtpDir &dir, FSEntryList& list) const;

    /**
     * @brief convert Converts the file flags on the remote system
     * into the set of file flags implemented within the MUSE
     * implementation of the FileSystem (FS) classes.
     * @param sftpFlags the sftp flags to be converted into the MUSE
     * implementation.
     * @return The MUSE representaiton of the sftp flags.
     */
    int  convert(const int sftpFlags) const;

public slots:
    /**
     * @brief moveToThread Moves the given ssh socket to a given
     * QThread. This method will call moveToThread(QThread, SshSocket).
     * This slot is needed from an object oriented perspective with
     * the FSMAsyncHelper.
     * @param thread The thread that the SshSocket will be moved to.
     */
    void moveToThread(QThread* thread);

protected slots:
    /**
     * @brief moveToThread Moves the given ssh socket to a given
     * QThread.
     * @param thread The thread that the SshSocket will be moved to.
     * @param ssh The SshSocket to be moved to a different thread.
     */
    void moveToThread(QThread* thread, SshSocket *ssh);


signals:
    void needSocket(QThread *thread, SshSocket *ssh);

private:
    SshSocket* const ssh;
    SFtpChannel* sftp;
    const bool deleteSocket;
    const FSEntry root;
};

#endif // REMOTEFSHELPER_H
