#ifndef LOCAL_SERVER_SESSION_H
#define LOCAL_SERVER_SESSION_H

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
#include <stdio.h>
#include <QFile>

/**
 * @brief A local server session to run jobs on the local host.
 * <p>This class provides an implementation of the ServerSession API.
 * Specifically, this class provides a session that can be used to
 * interact with the local PC using the same API as that used for
 * remote hosts. The consistent API eases development of the core GUI
 * modules.</p>
 */
class LocalServerSession : public ServerSession{
public:
    /**
     * @brief Creates a LocalServerSession by passing the parameters to the
     * super class, which initializes the variables.
     *
     * @param server The necessary information to connect to the server.
     * @param parent The parent GUI componenet that should be used to
     * create GUI elements that may be needed for any interactive
     * operations.
     */
    LocalServerSession(Server &server, QWidget *parent = NULL, QString purpose = "");

    /**
     * @brief Connect to the server to perform various operations.
     * This method is only useful for connections to non-local servers,
     * so the implementation here is empty.
     */
    void connectToServer();

    /**
     * @brief Method to disconnect from a remote server. Since the connect
     * method has an empty implementation, this method has an empty
     * implementation as well.
     */
    void disconnectFromServer();

    /**
     * @brief Executes to run a <b>brief</b> command that produces succint output.
     * The two possible outputs (standard output and error output) will be stored separately as Strings.
     *
     * @param command The command to be executed on the target machine.
     * The command must be compatible with the target machine's OS,
     * otherwise an exception will be generated.
     *
     * @param stdoutput The output that comes from the standard output
     * stream as a result of running the command.
     *
     *  @param stderrmsgs The output that comes from the standard error
     * stream as a result of running the command.
     *
     *  @return The exit code from the command that was run on the target machine.
     */
    int exec(const QString &command, QString &stdoutput,
              QString &stderrmsgs);

    /**
     * @brief exec Method to be run with a long running command that may
     * produce verbose output. The output is deposited in a QTextDocument
     * with appropriate styling given to the output returned from the command.
     * In other words, the standard output will appear differently than the
     * error stream and any other type of output.
     *
     * @param command The command to be executed on the target machine.
     * The command must be compatible with the target machine's OS,
     * otherwise an exception will be generated.
     *
     * @param output The QTextEdit that the output will be directed to and deposited in.
     *  This parameter cannot be null.
     *
     * @return The exit code from the command that was run on the target machine.
     */
    int exec(const QString &command,  QTextEdit &output);

    /**
     * @brief copy A method to copy given data from an input stream to a given file on the server.
     * @param srcData The source stream that provides the data to be copied.
     *
     * @param destDirectory The destination directory to which the data is to be copied.
     * This method will assume the directory has already been created.
     *
     * @param destFileName The name of the destination file to which the data will be copied to.
     *
     * @param mode The POSIX compliant mode string (such as: "0600" or "0700") to be used
     * as the mode for the target file.
     */
    void copy(const QString &srcData, const QString &destDirectory,
              const QString &destFileName, const int &mode);

    //Java version of below method also had a progress bar as a last parameter.....
    /**
     * @brief copy Copy file from a remote machine to a given output stream.
     * @param destData The destination stream to which the data is to be written.
     * @param srcDirectory The source directory from where the file is to be copied.
     * @param srcFileName The name of the source file from where the data is to be copied.
     */
    void copy(const QString &destData, const QString &srcDirectory,
              const QString &srcFileName);

    /**
     * @brief mkdir Creates a directory on the target machine.
     * This method must be used to create a directory entry on the server.
     * This method emits directoryCreated(bool) to announce whether or
     * not the directory was created.
     * @param directory The full path to the directory that is to be created.
     */
    void mkdir (const QString &directory);

    /**
     * @brief rmdir Removes an <i>empty</i> directory from the target machine.
     * This method will only succeed if the directory is empty. This method
     * emits directoryRemoved(bool) to announce whether or not the
     * directory was removed.
     * @param directory The full path to the empty directory to be removed from the target machine.
     */
    void rmdir(const QString &directory);

    /**
     * @brief fstat Obtain information about a given path on the target machine. This method uses SFTP to copy the data.
     * @param path The path (absolute or relative to home directory) of the file whose meta data is to be retrieved.
     * @return A FileInfo object containing the information about the path.
     */
    FileInfo fStat(const QString &path);

    /**
     * @brief setPurpose Sets a purpose message for this session.
     * @param text The purpose for this session. The text can contain
     * HTML tags or possibly an icon, so it can be HTML in addition to plain text.
     * If the message is long, then ensure it is properly broken into multiple lines
     * so that the dialog boxes display at reasonable sizes.
     */
    void setPurpose(const QString &text);

    /**
     * @brief Gets the purpose of this server session.
     * @return The purpose of this server session. If a purpose has not been set, then
     * null is returned.
     */
    QString& getPurpose();


protected:


private:
    QString& purpose;

    /**
     * @brief Set the permissions for a given file. This method uses
     * a given digit in the permission string to setup the read, write,
     * and execute information for a given file.
     *
     * @param file The file for which the permissions are to be set.
     * @param permDigit A POSIX compliant digit (0-7) that indicates the flags
     * for read (4), write (2), and execute (1) values.
     *
     * @param owner Flag to to indicate if the status is for the owner (true) or
     * for others (false).
     */
    void setPerms(QFile &file, const char &permDigit, const bool owner);

    //Not sure on this yet....
//    Process startProcess(QString *command);
};

#endif
