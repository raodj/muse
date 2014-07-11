#ifndef PROJECT_LIST_H
#define PROJECT_LIST_H

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

#include <QList>
#include "Project.h"

/**
 * @brief The ProjectList class A class that contains a list of projects
 * that can be defined by XMLElement for use in the MUSE_GUI workspace
 * file. This class contains a listing of MUSE projects as they pertain
 * to a specific server.
 */
class ProjectList : public XMLElement {
public:
    /**
     * @brief ProjectList A simple, default constructor that creates an
     * empty object. This is necessary for marhsalling and unmarshalling
     * the data to XML.
     */
    ProjectList();

    /**
     * @brief addProject Adds a project entry to the Server this
     * ProjectList belongs to.
     * @param entry The project to add.
     */
    void addProject(Project& entry);

    /**
     * @brief size Returns the number of projects in this ProjectList
     * @return the number of projects.
     */
    int size() const { return projectList.size(); }

    /**
     * @brief get Gets a non-constant reference to the Project defined at
     * index <i>i</i> of the project list. This method does not perform
     * any checks on the validity of the index given as a parameter, so it
     * is the responsibility of the caller of this method to do so.
     * @param i The index of the project.
     * @return A reference to the project.
     */
    Project& get (const int i) { return *dynamic_cast<Project*>(projectList[i]); }

    /**
     * @brief get Gets a constant reference to the Project defined at
     * index <i>i</i> of the project list. This method does not perform
     * any checks on the validity of the index given as a parameter, so it
     * is the responsibility of the caller of this method to do so.
     * @param i The index of the project.
     * @return A reference to the project.
     */
    const Project& get(const int i) const
    { return *dynamic_cast<const Project*>(projectList[i]); }

protected:
    QList<XMLElement*> projectList;
};

#endif
