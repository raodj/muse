#ifndef PBS_JOB_FILE_CREATOR_CPP
#define PBS_JOB_FILE_CREATOR_CPP

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

#include "PBSJobFileCreator.h"
#include <QFile>
#include <QTextStream>

PBSJobFileCreator::PBSJobFileCreator(QString pJobName, int hoursRunTime,
                                     int mem, QString memMod, int nodes, int ppn,
                                     QString pArgs, QString execFilePath,
                                     QString execName, bool wantEmail) {
    userWantsEmail = wantEmail;
    cmdLineLanguage = "#!/bin/bash\n\n";
    jobName = "#PBS -N " + pJobName;
    runTime = "#PBS -l walltime=" + QString::number(hoursRunTime) + ":00:00";
    memory =  "#PBS -l mem=" + QString::number(mem) + memMod;
    nodesAndPpn = "#PBS -l nodes=" + QString::number(nodes) + ":ppn=" +
            QString::number(ppn);
    args = "args=\"" + pArgs + "\"";
    startJob = "time mpiexec ./" + execName + " $args";
    echoStartOfJob = "echo -e \"" + startJob + "\"";
    echoLineDivider = "echo -e \"###############################\"";
    if (userWantsEmail) {
        email = "#PBS -m abe";
    }
    cd = "cd " + execFilePath;
    pbsBash = "#PBS -S /bin/bash";
}

bool
PBSJobFileCreator::saveToFile(QString fileName) {
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QTextStream out(&file);
        QString newLine = "\n";

        out << cmdLineLanguage << jobName << newLine << runTime << newLine
               << memory << newLine << nodesAndPpn << newLine << pbsBash
               << newLine;

        if (userWantsEmail) {
            out << email << newLine;
        }
        out << cd << newLine << args << newLine << echoStartOfJob << newLine <<
               echoLineDivider << newLine << startJob;
        file.close();
        return true;
    }
    return false;
}

#endif
