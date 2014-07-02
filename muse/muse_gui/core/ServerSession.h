#ifndef SERVER_SESSION_H
#define SERVER_SESSION_H

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


#include <iostream>
#include <ostream>
#include <QWidget>
#include <QString>
#include <QTextEdit>
#include "Server.h"

class FileInfo;

/**
 * @brief A base class for local and remote connections.
 *
 * This is a non-instantiable base class that serves as a common interface for
 * both local and remote server sessions. Local server sessions will run on the
 * local machine, while remote sessions will run the commands on a remote
 * machine via a secure shell (SHH) protocol.
 *
 * @see LocalServerSession
 * @see RemoteServerSession
 */

class ServerSession : public QObject {
    Q_OBJECT
public:

    /**
      * The constructor merely initializes the instance variables to
      * the values supplied as parameters.
      *
      * @param server The server data entry that provides the necessary
      * information to connect to the server.
      * @param parent The parent component that should be used to
      * create GUI elements that may be needed for any interactive
      * operations.
      */
    ServerSession(Server &server, QWidget *parent = NULL, QString osType = "");

    /**
     * @brief connect Method that establishes a connection to a server.
     *
     * This method must be used to establish a connection to a server before
     * performing any tasks. Being a virtual method, this method has no implementation
     * in the ServerSession class and is overridden in the derived classes.
     */
    virtual void connectToServer() = 0;

    /**
     * @brief disconnect Method that closes the connection to a server.
     *
     * This method must be used to end the existing connection to server.
     * Being a virtual method, this method has no implementation
     * in the ServerSession class and is overridden in the derived classes.
     */
    virtual void disconnectFromServer() = 0;

    /**
     * @brief exec Method to run a <b>brief</b> command that produces succint output.
     * The two possible outputs (standard output and error output) will be stored separately as Strings.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
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
    virtual int exec(const QString &command, QString &stdoutput, QString &stderrmsgs) = 0;

    /**
     * @brief exec Method to be run with a long running command that may
     * produce verbose output. The output is deposited in a QTextDocument
     * with appropriate styling given to the output returned from the command.
     * In other words, the standard output will appear differently than the
     * error stream and any other type of output.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
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
    virtual int exec(const QString &command, QTextEdit &output) = 0;

    /**
     * @brief copy A method to copy given data from an input stream to a given file on the server.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
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
    virtual void copy(std::istream &srcData, const QString &destDirectory,
                      const QString &destFileName, const QString &mode) = 0;


    //Java version of below method also had a progress bar as a last parameter.....
    /**
     * @brief copy Copy file from a remote machine to a given output stream.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
     * @param destData The destination stream to which the data is to be written.
     * @param srcDirectory The source directory from where the file is to be copied.
     * @param srcFileName The name of the source file from where the data is to be copied.
     */
    virtual void copy(std::ostream &destData, const QString &srcDirectory,
                      const QString &srcFileName) = 0;

    /**
     * @brief mkdir Creates a directory on the target machine.
     * This method must be used to create a directory entry on the server.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
     *
     * @param directory The full path to the directory that is to be created.
     */
    virtual void mkdir(const QString &directory) = 0;

    /**
     * @brief rmdir Removes an <i>empty</i> directory from the target machine.
     * This method will only succeed if the directory is empty.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
     * @param directory The full path to the empty directory to be removed from the target machine.
     */
    virtual void rmdir(const QString &directory) = 0;

    //FileInfo is a PEACE class that has yet to be translated to C++
    /**
     * @brief fstat Obtain information about a given path on the target machine. This method uses SFTP to copy the data.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
     * @param path The path (absolute or relative to home directory) of the file whose meta data is to be retrieved.
     * @return A FileInfo object containing the information about the path.
     */
    //virtual FileInfo fstat(const QString &path) = 0;

    /*Server class not yet translated to C++, so this method header may change
    once the Server class exists and the enumerations are transferred. Maybe it should
   return an int? */
    //virtual Server.OSType getOSType()= 0;

    /**
     * @brief setPurpose Sets a purpose message for this session.
     * @param text The purpose for this session. The text can contain
     * HTML tags or possibly an icon, so it can be HTML in addition to plain text.
     * If the message is long, then ensure it is properly broken into multiple lines
     * so that the dialog boxes display at reasonable sizes.
     */
    virtual void setPurpose(const QString &text) = 0;

    /**
     * @brief getServer Gets a pointer to the server this RemoteServerSession refers to.
     * @return A pointer to the server.
     */
    Server& getServer() const { return server; }

signals:
    /**
     * @brief directoryCreated Anounces whether or not a directory was
     * created by mkdir().
     * @param result True if the directory was sucessfully created, false
     * otherwise.
     */
    void directoryCreated(bool result);

    /**
     * @brief directoryRemoved Announces whether or not a directory was
     * removed by rmdir().
     * @param result True if the directory was sucessfully removed, false
     * otherwise.
     */
    void directoryRemoved(bool result);

protected:
    Server& server;
    const QWidget* parent;
    QString& osType;

};

#endif
