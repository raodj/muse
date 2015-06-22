#ifndef SERVER_LIST_H
#define SERVER_LIST_H

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
#include "Common.h"

/**
 * A class to encapsulate information about a list of servers that have
 * already been configured in this work space. This class is instantiated
 * from the Workspace class. This class is relatively straightforward in
 * that it merely contains a list of Server objects. In addition, it
 * facilitates marshalling and unmarshalling of server data via the
 * XMLElement base class and associated classes.
 *
 */
class ServerList : public XMLElement {
    Q_OBJECT
public:
    /**
     * @brief ServerList The default constructor.
     *
     * The default constructor creates an empty object, i.e., it does not
     * contain any server entries in it. The default constructor is required
     * by the type system for unmarshaling XML.
     */
    ServerList();

//    ServerList(const ServerList &list);

    int getListSize() const { return servers.size(); }

    /**
     * @brief ~ServerList The destructor.
     *
     * This class does not explicitly use any dynamic memory and consequenlty
     * the destructor does not have any specific functionality. It is present
     * as a place holder (for any future extensions) and to adhere to coding
     * conventions.
     *
     * \note The base class frees the copies of server entries managed
     * by this object.
     */
    virtual ~ServerList() {}

    /**
     * @brief addServer Add a new sever entry to the server list.
     *
     * This method does not perform any validation on the server entry
     * being added. Consequently, it is up to the caller to ensure that
     * the entry being added is valid (if needed).
     *
     * \note This method creates a copy of the server entry.
     *
     * @param entry The server entry to be added.
     */
    void addServer(const Server& entry);

    /**
     * @brief clear Clear all servers in the list
     */
    void clear();

    /**
     * @brief size Returns the number of server entries in this list.
     *
     * @return The number of server entries. This method returns zero
     * if the list is empty.
     */
    int size() const { return servers.size(); }

    /**
     * @brief get Obtain the server entry at a given index location.
     *
     * \note Calling this method with an invalid index value will have
     * undesierable side effects as this method does not perform any
     * special sanity checks on the index.
     *
     * @param i The index position at which the server entry is to be
     * returned.
     *
     * @return A reference to the server entry at the given location.
     */
    Server& get(const int i) { return *dynamic_cast<Server*>(servers[i]); }

    Server getServer(const int i) { return *dynamic_cast<Server*>(servers[i]); }

    /**
     * @brief getIndex Obtain index position of a given entry.
     *
     * @param serverID The ID of the server whose index position is
     * to be returned.
     *
     * @return The index position of the server entry if found. If the
     * specified ID is not found, then this method returns -1.
     */
    int getIndex(const QString& serverID) const;

    /**
     * @brief get Obtain the server entry at a given index location.
     *
     * \note Calling this method with an invalid index value will have
     * undesierable side effects as this method does not perform any
     * special sanity checks on the index.
     *
     * @param i The index position at which the server entry is to be
     * returned.
     *
     * @return A reference to the server entry at the given location.
     */
    const Server& get(const int i) const
    { return *dynamic_cast<const Server*>(servers[i]); }

signals:
    /** @brief Signal to indicate that information in the server
     * list has changed.
     *
     * This signal is emitted whenever servers are added, removed, or the
     * information is modified.
     *
     * @param change The type of change that has ocurred.
     *
     * @param start The starting index position for the change.
     *
     * @param end The ending index position for the change. Start is always
     * less than or equal to end.
     */
    void serverChanged(ChangeKind change, int start, int end);

private slots:
    /**
     * @brief serverUpdated Slot to intercept updates to server and
     * fire a serverChanged signal.
     *
     * This slot is used to propagate changes to underlying server object
     * up the hierarchy to various listeners appropriately.
     *
     * @param server The server whose information has been updated.
     */
    void serverUpdated(const Server& server);

protected:
    /**
     * @brief servers The list of server objects encapsulated by this object.
     *
     * This vector holds the actual server objects encapsulated by this
     * ServerList object. Operations on the list are performed via suitable
     * public API methods.
     */
    QList<XMLElement*> servers;

private:
    // Currently this class does not have any private members.
};

// Define custom template specialization class
DECLARE_METATYPE_HELPER(ServerList)

// Declare additional information required by Qt's meta type system
Q_DECLARE_METATYPE(ServerList)

#endif // SERVER_LIST_H
