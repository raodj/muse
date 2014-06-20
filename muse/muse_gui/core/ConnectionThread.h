#ifndef CONNECTION_THREAD_H
#define CONNECTION_THREAD_H

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

#include <QThread>
#include "SshSocket.h"
#include "ThreadedConnectionGUI.h"
#include "Server.h"

/**
 * @brief The ConnectionThread class A thread that holds the SshSocket
 * for the RemoteServerSession so that the connection can be non-blocking
 * to the MUSE GUI system.
 */
class ConnectionThread : public QThread {
    Q_OBJECT
public:
    /**
     * @brief ConnectionThread The primary constructor for the ConnectionThread.
     * Uses the Server reference so that it can pass it to ThreadedConnectionGUI
     * as well as pass Server information to the socket when it comes time to
     * connect to the server.
     * @param server The server that will be connected to.
     * @param parent
     */
    ConnectionThread(Server& server, QObject *parent = 0);
    ~ConnectionThread();

    /**
     * @brief run Starts the connection process to the Server. Before calling
     * SshSocket::connectToHost(), appropriate signals are connected to the
     * ThreadedConnectionGUI so that any necessary dialog messages can appear
     * and be responded to.
     */
    void run();

    /**
     * @brief getSocket Returns a pointer to the SshSocket being used for this
     * RemoteServerSession.
     * @return The SshSocket being used for this RemoteServerSession.
     */
    SshSocket* getSocket();

signals:
    /**
     * @brief exceptionThrown Alerts the main Qt gui thread that an
     * exception was thrown and that the program needs to display a
     * warning to the user explaining what happened.
     * @param e The exception thrown.
     */
    void exceptionThrown(const QString& message, const QString& genErrorMessage,
                         const QString& exceptionDetails);

    /**
     * @brief connectionResult Announces the result of the attempt to connect to
     * the remote server
     * @param result The result of the attempted connection.
     */
    void connectionResult(const bool result);

public slots:

private:
    SshSocket* connection;
    ThreadedConnectionGUI threadGUI;
    Server server;

};

#endif
