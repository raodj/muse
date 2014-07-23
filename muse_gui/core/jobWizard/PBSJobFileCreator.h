#ifndef PBS_JOB_FILE_CREATOR_H
#define PBS_JOB_FILE_CREATOR_H

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

#include <QString>

/**
 * @brief The PBSJobFileCreator class Class that creates a PBS job
 * script file for the user. These script files are usable by a computer
 * that supports the PBS task scheduling system.
 */
class PBSJobFileCreator {
public:
    /**
     * @brief PBSJobFileCreator Creates all of the commands that will go
     * into the script when save() is called.
     * @param pJobName The name of the job.
     * @param hoursRunTime The maximum amount of time this job is allowed
     * to run.
     * @param mem The amount of memory the program can use per processor.
     * @param memMod The data size of mem. Either MB or GB.
     * @param nodes The number of nodes to be used.
     * @param ppn The number of processors per node to be used.
     * @param pArgs The command line arguments needed to run the program.
     * @param execFilePath The file path to the executable file.
     * @param execName The name of the executable file
     * @param wantEmail If the user wants email notifications (true) sent
     * to them when the program starts and ends, or not (false).
     */
    PBSJobFileCreator(QString pJobName, int hoursRunTime, int mem, QString memMod, int nodes,
                      int ppn, QString pArgs, QString execFilePath,
                      QString execName, bool wantEmail);

    /**
     * @brief saveToFile Saves this job file to a text file.
     * @param fileName The name and path of the location to save this file.
     * @return True if the file was saved successfully, false otherwise.
     */
    bool saveToFile(QString fileName);

private:
    QString cmdLineLanguage;
    QString jobName, runTime, memory, nodesAndPpn, cmdLangForProgram,
    cd, args, echoStartOfJob, echoLineDivider, startJob, email, pbsBash;
    bool userWantsEmail;
};

#endif
