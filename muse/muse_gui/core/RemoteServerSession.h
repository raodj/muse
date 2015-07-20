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
    RemoteServerSession(Server server, QWidget* parent = nullptr);

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
     * projects, jobs, etc. and the scripts for getting server updates
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
};

#endif // REMOTESERVERSESSION_H
