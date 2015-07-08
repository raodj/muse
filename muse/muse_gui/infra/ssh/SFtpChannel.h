#ifndef SFTP_CHANNEL_H
#define SFTP_CHANNEL_H

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

#include "SFtpDir.h"

// Forward declaration to avoid circular dependency
class SshSocket;
class SFtpChannel;

/**
 * @brief The SFtpChannel class A MUSE class that eases the use of the libssh2
 * library by using single methods to call on a series of libssh2 methods.
 */
class SFtpChannel : public QObject {
    Q_OBJECT
public:
    /**
     * @brief SFtpChannel Instantiates a new channel of communications by
     * initializing a libssh2 sftp session with ssh SshSocket.
     * @param ssh The SshSocket through which this channel will be communicating
     * and working.
     */
    SFtpChannel(SshSocket& ssh);
    ~SFtpChannel();

    /**
     * @brief SFtpChannel Default copy constructor. This is required so
     * that this class can be used with RSSAsyncHelper, and QObject's
     * copy constructor is marked as deleted by the compiler
     * @param channel The SFtpChannel to copy.
     */
    SFtpChannel(const SFtpChannel& channel);

    /**
     * @brief getPwd Gets the path to the current working directory on this
     * SFtpChannel.
     * @return A QString representation of the path to the current working
     * directory.
     */
    QString getPwd();

    /**
     * @brief getErrorMessage Gets the error message description in the event
     * that an error occurs through the use of this SFtpChannel.
     * @return The error message.
     */
    QString getErrorMessage() const;

    /**
     * @brief getSocket Returns the SshSocket being used for this SFtopChannel.
     * @return The SshSocket
     */
    SshSocket& getSocket() const { return ssh; }

    /**
     * @brief getChannel Return the LIBSSH2_SFTP channel that is being used for
     * this SFtpChannel.
     * @return The SFtpChannel.
     */
    LIBSSH2_SFTP* getChannel() const { return sftpChannel; }

    /**
     * @brief getDir Creates a reference to an SFtpDir dir if dir exists.
     * @param dir The path to the directory that is to be operated on.
     * @return A reference to the SFtpDir created.
     */
    SFtpDir getDir(const QString& dir);

    /**
     * @brief mkdir Creates a directory on the target machine.
     * This method must be used to create a directory entry on the server.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method before calling this method.</p>
     *
     * @param directory The full path to the directory that is to be created.
     * @return True if the directory was successfully created, false otherwise.
     */
    bool mkdir(const QString& dir);

    bool mkdirs(const QStringList& dirs);

    bool validateServer(const QString& dir);

    /**
     * @brief rmdir Removes an <i>empty</i> directory from the target machine.
     * This method will only succeed if the directory is empty.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
     * @param directory The full path to the empty directory to be removed from the target machine.
     * @return True if the directory was successfully deleted, false otherwise.
     */
    bool rmdir(const QString& dir);

    /**
     * @brief dirExists Test if a given directory exists on the server.
     *
     * @param dir The directory to test
     *
     * @return True if the directory exists, false otherwise
     */
    bool dirExists(const QString& dir);

signals:
    /**
     * @brief alertUser Alerts the program that a message needs to be displayed
     * to the user.
     * @param message The message to display.
     */
    void alertUser(const QString& message);

protected:

private:
    SshSocket& ssh;
    LIBSSH2_SFTP* sftpChannel;
    int error;
};

#endif // SFTP_H
