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

#include "Server.h"

#include <QWidget>
#include <QString>
#include <QTextEdit>

#include <iostream>
#include <ostream>
#include <thread>

//class FileInfo;

enum class ChangeType {
    CONNECT,
    GET_OS_TYPE,
    CREATE_DIR,
    DIR_EXISTS,
    CREATE_SERVER,
    VALIDATE_SERVER,
};

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
      *
      * @param parent The parent component that should be used to
      * create GUI elements that may be needed for any interactive
      * operations.
      */
    ServerSession(Server server, QWidget* parent = nullptr);

    /**
     * @brief manageServer Main function called by the GUI to manage the
     * Server object owned by this ServerSession.  This will run the requested
     * command in a separate thread.
     *
     * @param change The type of change the user wants to make to the Server
     */
    virtual void manageServer(ChangeType change) final;

    /**
     * @brief setDirectory Set which Server directory we will be working with.
     * We do this so we don't have to pass a QString to every function.
     *
     * @param dir The directory to use
     */
    virtual void setDirectory(const QString& dir) final {
        directory = dir;
    }

    /**
     * @brief getServer Gets a pointer to the server this ServerSession refers to.
     *
     * @return A pointer to the server.
     */
    Server* getServer() const { return const_cast<Server*>(&server); }

signals:
    /**
     * @brief connectedToServer Annouces whether or not we have successfully
     * connected to the server
     *
     * @param result True if connected, false otherwise
     */
    void connectedToServer(bool result);

    /**
     * @brief announceOSType Announces what the operating system running on the
     * server is.
     *
     * @param os The name of the OS typedef
     *
     * @see Server
     */
    void announceOSType(QString os);

    /**
     * @brief directoryCreated Anounces whether or not a directory was
     * created by mkdir().
     *
     * @param result True if the directory was sucessfully created, false
     * otherwise.
     */
    void directoryCreated(bool result);

    /**
     * @brief directoryExists Announces whether or not a directory was
     * determined to exist by dirExtists()
     *
     * @param result True if the directory exists, false otherwise.
     */
    void directoryExists(bool result);

    /**
     * @brief serverDataCreated Announces whether or not createServerData()
     * successfully created the necessary Server data
     *
     * @param result True if the data was created, false otherwise
     */
    void serverDataCreated(bool result);

    /**
     * @brief directoryValidated Announces whether or not a directory was
     * determined to be a valid Server by validate()
     *
     * @param result True if the directory is a valid Server, false otherwise
     */
    void directoryValidated(bool result);

    /**
     * @brief errorEncountered Announces to the caller that an error has
     * been encountered while managing the server
     *
     * @param error The error message that occurred
     */
    void errorEncountered(QString error);

protected:
    Server server;

    QString directory;

    static const QString projectsDirName;
    static const QString jobsDirName;
    static const QString scriptsDirName;

    /**
     * @brief connect Method that establishes a connection to a server.
     *
     * This method must be used to establish a connection to a server before
     * performing any tasks.
     */
    virtual void connectToServer() = 0;

    /**
     * @brief getOSType Get the type of operating system running on the server.
     * This will be one of the Types found in the Server class
     *
     * @see Server
     */
    virtual void getOSType() = 0;

    /**
     * @brief mkdir Creates a directory on the target machine.
     * This method must be used to create a directory entry on the server.
     * <p><b>Note:</b>  The connection to the remote server must have
     * been established successfully via a call to connect method.</p>
     */
    virtual void mkdir() = 0;

    /**
     * @brief dirExists Tests if a given directory actually exists on the server.
     */
    virtual void dirExists() = 0;

    /**
     * @brief createServerData Attempts to create the necessary data for a Server
     * in the given directory.  This data includes xml files, directories for
     * projects, jobs, etc. and the scripts for getting server updates
     */
    virtual void createServerData() = 0;

    /**
     * @brief validate Checks if the given directory has the necessary files
     * and directories to be a valid Server
     */
    virtual void validate() = 0;

private:
    std::thread thread;
};

#endif
