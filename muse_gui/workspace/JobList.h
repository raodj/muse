#ifndef JOB_LIST_H
#define JOB_LIST_H

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

#include "XMLElement.h"
#include "Job.h"

/**
 * @brief The JobList class A class to encapsulate information about a list
 * of Jobs that have already been configured in this workspace. This class is
 * instantiated from the Workspace class. It facilitates the marshalling and
 * unmarshalling of Job data via the XMLElement base class.
 */
class JobList : public XMLElement {
public:
    JobList();

    /**
     * @brief addJob Add a new job entry to the job list.
     *
     * This method does not perform any validation on the job entry
     * being added. Consequently, it is up to the caller to ensure that
     * the entry being added is valid (if needed).
     *
     * \note This method creates a copy of the job entry.
     *
     * @param entry The job entry to be added.
     */
    void addJob(Job& job);

    /**
     * @brief size Returns the number of job entries in this list.
     * @return The number of job entries. Zero is returned if empty.
     */
    int size() const { return jobs.size(); }

    /**
     * @brief get Obtain the Job entry at a given index location.
     * @param i The index position of the Job entry to be returned.
     * @return  A reference to the job entry at the given location.
     *
     * \note This method does not perform any sanity checks on the
     * index passed to the parameter. It is the caller's responsibility
     * to ensure a valid index is given
     */
    Job& get(const int i) { return *dynamic_cast<Job*>(jobs[i]); }

    /**
     * @brief get Obtain the Job entry at a given index location.
     * @param i The index position of the Job entry to be returned.
     * @return  A reference to the job entry at the given location.
     *
     * \note This method does not perform any sanity checks on the
     * index passed to the parameter. It is the caller's responsibility
     * to ensure a valid index is given
     */
    const Job& get(const int i) const { return *dynamic_cast<const Job*>(jobs[i]); }

protected:
    QList<XMLElement*>jobs;
};

#endif // JOBLIST_H
