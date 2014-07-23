#ifndef JOB_H
#define JOB_H

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
#include <QString>
#include <QDateTime>

/**
 * @brief The Job class Contains information regarding a job to be executed
 * on a super computer. This data is contained in a MUSE Workspace.
 *
 * A Job entry (at the moment) contains the following information: the
 * Project it is operating on, the Server that this Job is running on,
 * the ID number provided by the super computer, the timestamp from
 * when this Job was submmited to the super computer, and the current
 * status of the job.
 */
class Job : public XMLElement {
public:
    /**
     * @brief Queued This status indicates that this Job is still in the queue
     * waiting to begin executing.
     */
    static const QString Queued;

    /**
     * @brief Running This status indicates that this Job is currently running.
     */
    static const QString Running;

    /**
     * @brief Exiting This status indicates that this Job is finishing its
     * execution.
     */
    static const QString Exiting;

    /**
     * @brief Complete This status indicates that this Job has either completed
     * or was cancelled by the user.
     */
    static const QString Complete;

    /**
     * @brief Job Default constructor with default values given to ease the
     * process of marshalling and unmarshalling data from the Workspace file.
     * @param pProject The name of the Project this job is running.
     * @param pServer The ID of the server this job will run on.
     * @param pJobId The ID number of this Job, given by the server.
     * @param pDateSubmitted The date and time this Job was submitted to the
     * server.
     * @param pStatus The status of the Job.
     */
    Job(QString pProject = "", QString pServer = "", long pJobId = 0,
        QDateTime pDateSubmitted = QDateTime::currentDateTime(),
        QString pStatus = Job::Queued);

    QString getProject() const { return project; }
    void setProject(const QString& projectName);

    QString getServer() const { return server; }
    void setServer(const QString& serverId);

    QString getStatus() const { return status; }
    void setStatus(const QString& currStatus);

    long getJobId() const { return jobId; }
    void setJobId(long id);

    QDateTime getDateSubmitted() const { return dateSubmitted; }
    void setDateSubmitted(const QDateTime& date);

private:
    QString project, server, status;
    long jobId;
    QDateTime dateSubmitted;
};

#endif // JOB_H
