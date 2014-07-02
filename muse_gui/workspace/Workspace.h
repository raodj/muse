#ifndef WORKSPACE_H
#define WORKSPACE_H

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

#include "XMLRootElement.h"
#include "ServerList.h"
#include <QDateTime>
#include "ServerListTableModel.h"
#include "Server.h"

/**
 * @brief The Workspace class that stores and manages all the data associated
 * with a MUSE-GUI workspace.
 *
 * <p>Note that the purpose of the Workspace class is to
 * encapsulate and manage the data associated with the workspace and not to
 * display it. Display of Workspace data is delegated to other classes that
 * focus on presenting a suitable "view" of the data. This design follows the
 * Model-View-Controller (MVC) design pattern.</p>
 *
 * <p>Currently, the GUI operates only with a single Workspace.
 * Consequently to enforce the use of a single, unique Workspace object, this
 * class has been designed using the Singleton design pattern.</p>
 *
 * <p>This class also extends the XMLRootElement to ease marshalling and
 * unmarshalling to/from XML for persistence.</p>
 */
class Workspace : public XMLRootElement {
public:
    /**
     * @brief get Returns a pointer to the process-wide, unique workspace
     * (if one is available).
     *
     * The workspace object is a singleton (there is only one object).
     * Making changes in one affects all of the views of the workspace.
     *
     * @return The process-wide, unique workspace instance. If a workspace
     * has not yet been created, then this method returns NULL.
     *
     */
    static Workspace* get() {
        return workspace;
    }

    /**
     * @brief ~Workspace The destructor.
     *
     * Currently, the destructor does not have any special operations to
     * perform because this class currently does not directly use any
     * dyanmically allocated objects
     */
    ~Workspace() {}

    /**
     * @brief good Determine if the workspace is a good condition.
     *
     * This flag is used to determine if the current workspace is
     * valid and ready for operations.
     *
     * @return This method returns true if the workspace is deemed to be
     * in a good condition. Otherwise this method returns false.
     */
    bool good() const { return isGood; }

    /**
     * @brief dumpWorkspace Dump workspace data in XML format to the
     * programmer log for verification.
     *
     * This is a convenience method that can be used to dump the workspace
     * in XML format to the programmer log. The XML data can be used for
     * various types of verification and troubleshooting operations.
     */
    void dumpWorkspace();

    /**
     * @brief getServerList Obtain the list of servers in this workspace
     *
     * @return This method returns the list of servers associated with this
     * workspace.
     */
    ServerList& getServerList() { return serverList; }

    /**
     * @brief createWorkspace Create a default workspace file in the
     * given directory.
     *
     * @param directory The directory to be used for creating the
     * workspace. This directory must be valid and should exist.
     *
     * @return This method returns an empty string if the workspace was
     * successfully created. On various errors this method returns a
     * non-empty string with suitable error message.
     */
    static QString createWorkspace(const QString& directory);

    /**
     * @brief useWorkspace Use an existing workspace by unmarhsalling
     * data from an XML file.
     *
     * @param directory The directory associated with the workspace to be
     * used for processing.
     *
     * @return This method returns an empty string if the workspace was
     * successfully opened. On various errors this method returns a
     * non-empty string with suitable error message.
     */
    static QString useWorkspace(const QString& directory);

    /**
     * @brief save Save the current workspace.
     *
     * This method must be used to save the current workspace to the
     * workspace file.
     *
     * \note On errors the good-status of the workspace is left unchanged
     * giving the user an opportunity to try to save again.
     *
     * @return This method returns an empty string if the workspace was
     * successfully saved. On various errors this method returns a
     * non-empty string with suitable error message.
     */
    QString saveWorkspace();

    /**
     * @brief reserveID Assigns a unique ID to an object in this
     * workspace.
     * @param itemType The type of item in the workspace that is
     * going to receive a unique ID.
     * @return The unique ID that itemType is assigned.
     */
    QString reserveID(const QString& itemType);

    /**
     * @brief getTableModel Returns the table model representation of this
     * workspace.
     * @return  The ServerListTableModel of this workspace.
     */
    ServerListTableModel& getTableModel();

    /**
     * @brief addServerToWorkSpace Adds server to the serverList and to the
     * ServerListTableModel.
     * @param server The server to be added.
     */
    void addServerToWorkSpace(Server& server);

protected:
    /**
     * @brief Workspace The constructor creates a dummy workspace object.
     *
     * This constructor is used to create a default workspace. The
     * constructor calls the registerClasses() method to add the
     * pertinent classes for marshalling XML data to Qt's type
     * system. In addition, the constructor adds default namespaces
     * and attributes.
     *
     * @param directory The directory associated with the workspace.
     * The constructor does not perform any special checks on the
     * directory.
     *
     * @param isValid Flag to indicate if the workspace is in a valid
     * state.
     */
    Workspace(const QString& directory = "", bool isValid = false);

    /**
     * @brief registerClasses Convenience helper method to register
     * classes for unmarshalling.
     *
     * This method is a helper method that was introduced to aggregate
     * registration of classes derived from XMLElement with Qt's
     * type system using the qRegisterMetaType method.
     */
    static void registerClasses();

private:
    /**
     * @brief directory The full path to the folder in which local files
     * associated with this workspace are stored.  This value is set when
     * a workspace is created.
     */
    QString directory;

    /**
     * @brief timestamp The timestamp when the workspace was last updated.
     * This information is updated each time the workspace information
     * is updated (it is typically unrelated to the last time the work
     * space was saved which would be the timestamp on the workspace
     * file).
     */
    QDateTime timestamp;

    /**
     * @brief seqCounter A monotonically increasing sequence counter that
     * is used to generate a unique IDs for various identifiers in this
     * workspace.
     */
    long seqCounter;

    /**
     * @brief serverList Object that encapsulates the list of servers
     * currently added to this workspace.
     */
    ServerList serverList;

    /**
     * @brief good Flag to indicate if the workspace is in a good state.
     * This flag is typically initialized to false when a dummy workspace
     * is created (at startup). This flag is set to true once a default
     * workspace is successfully created or a workspace is successfully
     * loaded from a given file.
     */
    bool isGood;

    /**
     * The process-wide, unique instance of the workspace object. This object
     * contains all the necessary information regarding the workspace that
     * is being currently used. The pointer is initialized to NULL. A valid
     * workspace object can be created via calls to createWorkspace and
     * useWorkspace static methods in this class.
     */
    static Workspace *workspace;

    /**
     * @brief WorkspaceFileName A constant to refer to the specific
     * workspace file.  This value is appended to a directory path
     * to determine the actual workspace XML file.
     */
    static const QString WorkspaceFileName;

    ServerListTableModel serverModel;

    /**
     * @brief addInitialServersToModel Iterates through serverList and adds
     * the servers to the table model for any views that use the
     * ServerListTableModel.
     */
    void addInitialServersToModel();
};

#endif // WORKSPACE_H
