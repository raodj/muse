#ifndef JOB_LIST_TABLE_CPP
#define JOB_LIST_TABLE_CPP

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

#include "JobListTableModel.hpp"

JobListTableModel::JobListTableModel() {
    //Set the column headers
    for (int i = 0; i < columnTitles.size(); i++) {
        setHeaderData(i, Qt::Horizontal, columnTitles[i], Qt::DisplayRole);
    }
}

QVariant
JobListTableModel::headerData(int section, Qt::Orientation orientation,
                              int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        // Ignore this type of request
        return QVariant();
    }

    return  columnTitles[section];
}

QVariant
JobListTableModel::data(const QModelIndex &index, int role) const {
    if ((index.row() < 0)    || (index.row() >= jobEntries.size()) ||
        (index.column() < 0) || (index.column() >= columnTitles.size())) {
        // A request that cannot be handled.
        return QVariant();
    }

    //Rest of method only executed if role is the display role.
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    //Get the desired row of the server list
    const Job &job = jobEntries.at(index.row());

    //Return the value corresponding to the column.
    switch (index.column()) {
    case 0: return job.getName();
    case 1: return job.getServer();
    case 2: return job.getStatus();
    case 3: return (long long int) job.getJobId(); // long int cant be used in QVariant
    default:
    case 4: return job.getDateSubmitted();
    }
}

void
JobListTableModel::appendJobEntry(Job& job) {
    // Let base class know a row is being added
    beginInsertRows(QModelIndex(), jobEntries.size(),
                    jobEntries.size() + 1);

    // Add the server to the list
    jobEntries.append(job);

    // We are done inserting rows
    endInsertRows();

    emit jobAdded();
}

#endif
