#ifndef JOB_CPP
#define JOB_CPP

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

#include "Job.h"

const QString Job::Queued = "queued";

const QString Job::Running = "running";

const QString Job::Exiting = "exiting";

const QString Job::Complete = "complete";

Job::Job(QString pName, QString pServer, long pJobId,
         QDateTime pDateSubmitted, QString pStatus, QString pDescription) :
    XMLElement("Job"), name(pName), server(pServer), jobId(pJobId),
    dateSubmitted(pDateSubmitted), status(pStatus), description(pDescription) {
    // Add the set of instance variables that must be serialized/deserialized
    addElement(XMLElementInfo("JobName", &name));
    addElement(XMLElementInfo("ID", &jobId));
    addElement(XMLElementInfo("Status", &status));
    addElement(XMLElementInfo("Description", &description));
    addElement(XMLElementInfo("ServerUsed", &server));
    addElement(XMLElementInfo("DateSubmitted", &dateSubmitted));

}

void
Job::setName(const QString& newName) {
    name = newName;
}

void
Job::setServer(const QString& serverId) {
    server = serverId;
}

void
Job::setStatus(const QString& currStatus) {
    status = currStatus;
}

void
Job::setJobId(long id) {
    jobId = id;
}

void
Job::setDateSubmitted(const QDateTime& date) {
    dateSubmitted = date;
}

void
Job::setDescription(const QString& desc) {
    description = desc;
}


#endif
