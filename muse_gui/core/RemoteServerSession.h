#ifndef REMOTE_SERVER_SESSION_H
#define REMOTE_SERVER_SESSION_H

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

#include "ServerSession.h"
#include "ThreadedConnectionGUI.h"
#include "SFtpChannel.h"
#include "SshChannel.h"

#include <memory>

/**
 * @brief A remote server session based on the secure shell (SSH) protocol.
 *
 * <p>This class provides an implementation of the ServerSession API.
 * Specifically, this class provides a session that can be used to
 * interact with a remote host via the secure shell (SSH) protocol.
 * The secure shell protocol is a defacto standard for interacting
 * with remote servers via the Internet today. It provides all the
 * necessary security features to safely interact with remote
 * hosts and almost all super computing clusters mandate the use of
 * SSH for interactions. </p>
 *
 * <p>This class uses the Ganymede SSH
 * implementation for establishing SSH connections. Ganymede SSH
 * supports only ssh-2 protocol. Please refer to Genymede SSH website
 * for further details: <A HREF="http://www.ganymed.ethz.ch/ssh2/">
 * http://www.ganymed.ethz.ch/ssh2/</A>. MUSE distributes Ganymede
 * license file as per Ganymede licensing requirements.</p>
 *
 */
class RemoteServerSession : public ServerSession {
    Q_OBJECT
public:

    /**
     * @brief Creates a RemoteServerSession by passing the parameters to the
     * super class, which initializes the variables.
     *
     * @param server The necessary information to connect to the server.
     * @param parent The parent GUI componenet that should be used to
     * create GUI elements that may be needed for any interactive
     * operations.
     */
    RemoteServerSession(const Server& server, QWidget* parent = nullptr);

//    /**
//     * @brief Disconnect from a remote server.
//     *
//     * This method disconnects from the remoter server if it is connected.
//     * All current sessions will be terminated.
//     */
//    void disconnectFromServer() override;

//    /**
//     * @brief Executes to run a <b>brief</b> command that produces succint output.
//     * The two possible outputs (standard output and error output) will be stored separately as Strings.
//     * <p><b>Note:</b>  The connection to the remote server must have
//     * been established successfully via a call to connect method before calling this method.</p>
//     *
//     * @param command The command to be executed on the target machine.
//     * The command must be compatible with the target machine's OS,
//     * otherwise an exception will be generated.
//     *
//     * @param stdoutput The output that comes from the standard output
//     * stream as a result of running the command.
//     *
//     *  @param stderrmsgs The output that comes from the standard error
//     * stream as a result of running the command.
//     *
//     *  @return The exit code from the command that was run on the target machine.
//     */
//    int exec(const QString &command, QString &stdoutput, QString &stderrmsgs) override;

//    /**
//     * @brief exec Method to be run with a long running command that may
//     * produce verbose output. The output is deposited in a QTextDocument
//     * with appropriate styling given to the output returned from the command.
//     * In other words, the standard output will appear differently than the
//     * error stream and any other type of output.
//     * <p><b>Note:</b>  The connection to the remote server must have
//     * been established successfully via a call to connect method before this method is called.</p>
//     *
//     * @param command The command to be executed on the target machine.
//     * The command must be compatible with the target machine's OS,
//     * otherwise an exception will be generated.
//     *
//     * @param output The QTextEdit that the output will be directed to and deposited in.
//     *  This parameter cannot be null.
//     *
//     * @return The exit code from the command that was run on the target machine.
//     */
//    int exec(const QString &command, QTextEdit& output) override;

//    /**
//     * @brief copy A method to copy given data from an input stream to a given file on the server.
//     * <p><b>Note:</b>  The connection to the remote server must have
//     * been established successfully via a call to connect method.</p>
//     * @param srcData The source stream that provides the data to be copied.
//     *
//     * @param destDirectory The destination directory to which the data is to be copied.
//     * This method will assume the directory has already been created.
//     *
//     * @param destFileName The name of the destination file to which the data will be copied to.
//     *
//     * @param mode The POSIX compliant mode string (such as: "0600" or "0700") to be used
//     * as the mode for the target file.
//     */
//    bool copy(const QString &srcData, const QString &destDirectory,
//              const QString &destFileName, const int &mode) override;

//    //Java version of below method also had a progress bar as a last parameter.....
//    /**
//     * @brief copy Copy file from a remote machine to a given output stream.
//     * <p><b>Note:</b>  The connection to the remote server must have
//     * been established successfully via a call to connect method before calling this method.</p>
//     * @param destData The destination stream to which the data is to be written.
//     * @param srcDirectory The source directory from where the file is to be copied.
//     * @param srcFileName The name of the source file from where the data is to be copied.
//     */
//    bool copy(const QString &destData, const QString &srcDirectory,
//              const QString &srcFileName) override;

//    /**
//     * @brief fstat Obtain information about a given path on the target machine. This method uses SFTP to copy the data.
//     * <p><b>Note:</b>  The connection to the remote server must have
//     * been established successfully via a call to connect method.</p>
//     * @param path The path (absolute or relative to home directory) of the file whose meta data is to be retrieved.
//     * @return A FileInfo object containing the information about the path.
//     */
//    //FileInfo fStat(const QString &path);

//    /**
//     * @brief setPurpose Sets a purpose message for this session.
//     * @param text The purpose for this session. The text can contain
//     * HTML tags or possibly an icon, so it can be HTML in addition to plain text.
//     * If the message is long, then ensure it is properly broken into multiple lines
//     * so that the dialog boxes display at reasonable sizes.
//     */
//    void setPurpose(const QString &text);

//    /**
//     * @brief Gets the purpose of this server session.
//     * @return The purpose of this server session. If a purpose has not been set, then
//     * null is returned.
//     */
//    QString& getPurpose();

//    /**
//     * @brief getSocket Returns a pointer to the SshSocket being used
//     * for this session.
//     * @return A pointer to the socket.
//     */
//    SshSocket* getSocket();

//    /**
//     * @brief openSftpChannel Opens the SFtpChannel for this server session.
//     */
//    void openSftpChannel();

//    /**
//     * @brief closeSftpChannel Closes the SFtpChannel for this server
//     * session.
//     */
//    void closeSftpChannel();

//    /**
//     * @brief closeSshChannel Closes the SshChannel for this server
//     * session.
//     */
//    void closeSshChannel();

//    /**
//     * @brief getSftpChannel Returns a pointer to the SFtpChannel
//     * being used for this server session.
//     * @return A pointer to the SFtpChannel.
//     */
//    SFtpChannel* getSftpChannel();

protected:

    /**
     * @brief Connect to the server in order to perfrom various operations.
     *
     * This method must be used to establish a connection to a server before
     * performing any tasks. This method is overridden from the base class.
     */
    void connectToServer() override;

    /**
     * @brief getOSType Get the type of operating system running on the server.
     * This will be one of the Types found in the Server class
     *
     * @see Server
     */
    void getOSType() override;

    /**
     * @brief mkdir Creates a directory on the target machine.
     * This method must be used to create a directory entry on the server.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method before calling this method.</p>
     *
     * @param directory The full path to the directory that is to be created.
     */
    void mkdir() override;

    /**
     * @brief dirExists Tests if a given directory actually exists on the server.
     *
     * @param directory The full path to the directory that you want to check
     */
    void dirExists() override;

    /**
     * @brief createServerData Attempts to create the necessary data for a Server
     * in the given directory.  This data includes xml files, directories for
     * projects, jobs, etc. and the server scripts for getting server updates
     *
     * @param directory The full path to the directory that we want to make a Server
     */
    void createServerData() override;

    /**
     * @brief validate Checks if the given directory has the necessary files
     * and directories to be a valid Server
     *
     * @param directory True if the directory is a valid Server, false otherwise
     */
    void validate() override;

private:
    ThreadedConnectionGUI threadGUI;

    SshSocket socket;
//    SFtpChannel* sftpChannel;
//    SshChannel* sshChannel;
//    SshSocket* socket;
    //QString& purpose;

    //bool threadedResult;
    //int numericThreadedResult;
};

#endif // REMOTESERVERSESSION_H