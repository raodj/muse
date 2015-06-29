#ifndef SERVER_H
#define SERVER_H

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

#include <XMLElement.h>
#include "ProjectList.h"
#include "Project.h"
#include "Job.h"

/**
 * @brief The Server class that encapsulates information regarding a Server entry
 * in a Workspace.
 *
 * A Server entry represents either a single stand alone machine or the head node
 * of a supercomputing cluster on which Jobs can be run. A server entry
 * encapsulates all the information needed to access the server and run jobs on it.
 * In addition, it also provides the necessary infrastructure for marshaling and
 * unmarshaling data for persisting the information in a XML configuration file.
 */
class Server : public XMLElement {
    Q_OBJECT
public:
    // Predefined constants consistent with XML schema values for the server status.
    /**
     * @brief Installing This server status indicates that the GUI is making
     * attempt to install the runtime subsystem of PEACE onto the server.
     * This is a
     * relatively long-running process and the server can be in this
     * state for 5-10 minutes initially. After that a server never
     * transitions to this state.
     */
    static const QString Installing;

    /**
     * @brief InstallFailed This status indicates that the installation process of
     * the server failed and the server is unusable.
     */
    static const QString InstallFailed;

    /**
     * @brief Good This status string indicates the server is good to go
     * and is ready for further use.
     */
    static const QString Good;

    /**
     * @brief Uninstalling This status string indicates that the GUI is in the
     * process of uninstalling the runtime subsystem of PEACE from the remote
     * machine. In this state, the server is not usable.
     */
    static const QString Uninstalling;

    /**
     * @brief UninstallFailed This state string indicates that the uninstall
     * attempt on the server has failed. The server is now in an undefined state
     * and the user must try to clean up the entry manually.
     */
    static const QString UninstallFailed;

    /**
     * @brief ConnectFailed The previous attempt to talk to the server failed
     * and the user needs to diagnose this server.
     */
    static const QString ConnectFailed;

    // Predefined constants for the server type.

    /**
     * @brief Linux This predefined string constant identifies the type of the OS
     * on this server to be Linux.
     */
    static const QString Linux;

    /**
     * @brief Unix This predefined string constant identifies the type of the OS
     * on this server to be Unix/BSD.
     */
    static const QString Unix;

    /**
     * @brief Windows This predefined string constant identifies the type of the OS
     * on this server to be Windows.
     */
    static const QString Windows;

    /**
     * @brief Windows This predefined string constant identifies the type of the OS
     * on this server to be Apple OS X.
     */
    static const QString OSX;

    /**
     * @brief UnknownOS This predefined string constant is used denote OS whose
     * actual type is not known.
     */
    static const QString UnknownOS;



     /**
     * @brief Server  The constructor merely initializes all the instance
     * variables using the supplied parameters.
     *
     * This constructor does not perform any specific sanity checks on the
     * parameters.  The constructor has default values set to aid unmarshalling
     * objects from XML. Consequently, it is the caller's responsibility to
     * ensure that the argument values have been validated piror to creating
     * a server entry. The parameter values are mostly obtained from the user
     * (via a suitable GUI dialog).
     *
     * @param ID A unique identifier for this Server entry. For new Server
     * entries  this value is obtained via the ServerList.reserveServerID() method.
     *
     * @param remote If this flag is true, then the entry is a remote server
     * entry. Otherwise the entry is for local machine.
     *
     * @param name The domain name (or IP address) to be used for accessing
     * this server. For local machine, this value is simply set to null.
     *
     * @param port The port number (meaningful only for remote servers)
     * over which secure connections are to be established. The default
     * value is 22.
     *
     * @param description A user-assigned description for this server entry.
     * The description can be anything the user chooses to assign.
     *
     * @param userID The login user ID to be used for accessing remote
     * clusters. For the local machine, this value is set to null.
     *
     * @param installPath The location on the Server where PEACE is installed
     * and the necessary runtime components of PEACE are located.
     *
     * @param osType The type of operating system running on the server.
     * This value must be one of the predefined operating system types.
     *
     * @param status The initial status for this server (if any). The server
     * status must be one of the predefined constant status values from this
     * class.
     */
    Server(QString ID = "", bool remote = false, QString name = "", int port = 22,
           QString description = "", QString userID = "", QString installPath = "",
           QString osType = UnknownOS, QString status = Installing);

    Server(const Server& server);
    //Server();

    bool operator==(const Server other);

    /**
     * @brief ~Server The destructor.
     *
     * This class does not explicitly use any dynamic memory and consequenlty the
     * destructor does not have any specific functionality. It is present as
     * a plase holder (for any future extensions) and to adhere to coding
     * conventions.
     */
    ~Server() {}

    /**
     * @brief getID Returns the workspace-unique ID assigned for this server
     * entry.
     *
     * This value is used to make cross references to this server entry in
     * other artifacts on the workspace.
     *
     * @return Return the workspace-unique ID assigned for this server entry.
     */
    QString getID() const { return ID; }

    /**
     * @brief setID Sets the unique id of this Server to id, which is obtained
     * from Workspace::reserveID().
     * @param id The id for this Server.
     */
    void setID(const QString& id);

    /**
     * @brief getName Returns the server's domain name (or IP address) set
     * for this server entry.
     *
     * The value returned by this method is typically the value that was set
     * when this server entry was created (as the name is never directly
     * changed).
     *
     * @return The server's domain name (or IP address).
     */
    QString getName() const { return name; }

    /**
     * @brief setName Set the name (or IP address) for this server.
     *
     * @param name A valid domain name (such as: redhawk.hpc.miamiOH.edu)
     * or IP address (192.168.0.1) for this server.
     */
    void setName(const QString &name);

    /**
     * @brief setName Set the name (or IP address) for this server.
     *
     * @param name A valid domain name (such as: redhawk.hpc.miamiOH.edu)
     * or IP address (192.168.0.1) for this server.
     */
    void setName(const std::string &name);

    /**
     * @brief getStatus Returns the server's status set for this server entry.
     *
     * @return The server's status
     */
    QString getStatus() const { return status; }

    /**
     * @brief getUserID Returns the userID set for this server entry.
     * @return  The userID credential.
     */
    QString getUserID() const { return userID; }

    /**
     * @brief getPort Returns the port number used to communicate with
     * this Server.
     * @return The port number.
     */
    int getPort() const { return port; }

    /**
     * @brief setPassword Sets the password for this Server entry.
     * @param credential The password.
     */
    void setPassword(const QString& credential);

    /**
     * @brief getPassword Gets the password for this Server entry.
     * In most cases, there will not be a password stored in this
     * Server entry, so the common return will be an empty QString.
     * @return
     */
    QString getPassword() const { return password; }

    /**
     * @brief setDescription Sets the description for this Server entry.
     * @param description The description to be set.
     */
    void setDescription(const QString& description);

    /**
     * @brief getDescription Gets the description for this Server entry.
     * @return The description for this Server entry.
     */
    QString getDescription() const { return description; }

    /**
     * @brief isRemote Returns whether or not the Server referred to
     * by this Server entry is a remote server.
     * @return True if this Server is a remote server, false otherwise.
     */
    bool isRemote() const { return remote; }

    /**
     * @brief setOS Sets the operating system for this Server.
     * @param os The operating system this Server is running.
     */
    void setOS(const QString& os);

    /**
     * @brief getOS Obtain the operating system of this server.
     *
     * @return The operating system that was detected (or set) for
     * this server.
     */
    QString getOS() const { return osType; }

    /**
     * @brief setInstallPath Sets the dirctory path containing the MUSE
     * system and its runtime components.
     * @param path The path to the directory where MUSE is installed.
     */
    void setInstallPath(const QString& path);

    /**
     * @brief getInstallPath Gets the install path for this Server entry.
     * @return The install path.
     */
    QString getInstallPath() const { return installPath; }

    /**
     * @brief addProject Adds the given project to the project list.
     * @param project The project to be added.
     */
    //void addProject(Project& project);

    /**
     * @brief getProjectList Gets the list of projects for this Server.
     * @return The list of Projects.
     */
    //ProjectList& getProjectList() { return projects; }

signals:
    /**
     * @brief serverUpdated Signal generated when information in this server
     * is updated.
     *
     * @param server The server whose information has been updated.
     */
    void serverUpdated(const Server& server);

protected:
    // Currently this class does not have any protected members.

private:
    /**
     * @brief ID A unique identifier for this Server entry.
     *
     * For new Server entries this value is obtained via the
     * Workspace.reserveID("Server") method.
     */
    QString ID;

    /**
     * @brief remote This instance variable indicates if this server entry
     * represents a local or a remote server. If this flag is true then
     * this entry corresponds to a remote server which will require a SSH
     * connnection to access.
     */
    bool remote;

    /**
     * @brief name The domain name (such as: redhawk.hpc.muohio.edu) or
     * IP address (such as: 134.53.13.131) to be used for accessing
     * the server. For local machine, this value is simply set to an
     * empty string.
     */
    QString name;

    /**
     * @brief port The port number over which remote servers are to be
     * contacted.
     *
     * For most traditional SSH installations, the default port is 22.
     * However, for non-traditional hosts, the port number can vary.
     * Varying the port number permits creation of tunnels etc. which
     * makes it convenient to work around fire walls or with multiple
     * clients.
     *
     * \note The port number is meaningful only for remote servers
     * whose remote flag is set to true.
     */
    int port;

    /**
     * @brief description A user-assigned description for this server entry.
     *
     * The description can be anything the user chooses. This is meant
     * to be meaningful only to the user.
     */
    QString description;

    /**
     * @brief userID The login ID to be used for logging into the remote
     * machine. A valid domain name (such as: redhawk.hpc.miamiOH.edu)
     * or IP address (192.168.0.1) for this server.
     */
    QString userID;

    /**
     * The location on the Server where PEACE is installed
     * and the necessary runtime components of PEACE are located.
     */
    QString installPath;

    /**
     * @brief status The current operational status of this server. This value
     * changes periodically as the server is installed, used, and
     * uninstalled.  The value in this variable is one of the predefined
     * constants Installing, InstallFailed, Good, Uninstalling,
     * UninstallFailed, or ConnectFailed.
     *
     */
    QString status;

    /**
     * @brief osType The operating system type for this server. This value
     * is determined when a new OS entry is added to a workspace.
     * This value is persisted in the workspace and restored when
     * a workspace is loaded.  The values are one of the predefined
     * constants Linux, Unix, or Windows.
     */
    QString osType;

    /**
     * @brief password This is a transient field that is never persisted
     *(for security purposes). Typically, it is set once (each time the GUI
     * is run) when an attempt is made to access the server.
     */
    QString password;

    //ProjectList projects;
};

// Define custom template specialization class
DECLARE_METATYPE_HELPER(Server)

// Declare additional information required by Qt's meta type system
Q_DECLARE_METATYPE(Server)

#endif // SERVER_H
