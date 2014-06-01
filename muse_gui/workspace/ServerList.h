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
public:
    /**
     * @brief ServerList The default constructor.
     *
     * The default constructor creates an empty object, i.e., it does not
     * contain any server entries in it. The default constructor is required
     * by the type system for unmarshaling XML.
     */
    ServerList();

    /**
     * @brief ~ServerList The destructor.
     *
     * This class does not explicitly use any dynamic memory and consequenlty
     * the destructor does not have any specific functionality. It is present
     * as a plase holder (for any future extensions) and to adhere to coding
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

#endif // SERVER_LIST_H
