#ifndef PROJECT_H
#define PROJECT_H

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
#include <QStringList>

/**
 * @brief The Project class Stores information about a project that
 * runs on a MUSE server.
 */
class Project : public XMLElement {
public:
    /**
     * @brief Project The constructor initializes all the instance variables
     * using the supplied parameters.
     * @param pName The name of the project.
     * @param pMakeFilePath The path to the make file for the project. This
     * path is on the Server this Project is associated with.
     * @param pExecutablePath The path to the executable for this Project. This
     * path is on the Server this Project is associated with.
     * @param pOutputDirPath The path to the output directory for this Project.
     */
    Project(QString pName = "",  QString pMakeFilePath = "",
            QString pExecutablePath = "", QString pOutputDirPath = "",
            long pJobCount = 0);

    ~Project() {}

    /**
     * @brief getSourceFileList Returns the QStringList of source files for this
     * Project.
     * @return The list of source files.
     */
    QStringList getSourceFileList() const { return sourceFileList; }

    /**
     * @brief setSourceFileList Sets the list of source files for this Project.
     * @param list The list that contains the new set of source files.
     */
    void setSourceFileList(const QStringList& list);

    /**
     * @brief getExecutablePath Gets the executable path for this Project.
     * @return The path to the executable
     */
    QString getExecutablePath() const { return executablePath; }

    /**
     * @brief setExecutablePath Sets the executable path to <i>path</i>
     * @param path The new executable path
     */
    void setExecutablePath(const QString& path);

    /**
     * @brief getOutputDirPath Gets the path to the output directory for
     * this Project.
     * @return The path to the output directory.
     */
    QString getOutputDirPath() const { return outputDirPath; }

    /**
     * @brief setOutputDirPath Sets the path to the output directory for
     * this project to <i>path</i>.
     * @param path The new path to the output directory.
     */
    void setOutputDirPath(const QString& path);

    /**
     * @brief getMakeFilePath Gets the path to the make file for this Project.
     * @return The path to the make file.
     */
    QString getMakeFilePath() const { return makeFilePath; }

    /**
     * @brief setMakeFilePath Sets the path to the make file for
     * this project to <i>path</i>.
     * @param path The new path to the make file.
     */
    void setMakeFilePath(const QString& path);

    /**
     * @brief getName Gets the name for this Project.
     * @return The name of this Project.
     */
    QString getName() const { return name; }

    /**
     * @brief setName Sets the name for this Project.
     * @param name The new name for this Project.
     */
    void setName(const QString& name);

    /**
     * @brief reserveJobId Returns a numeric identifier to be used
     * to be added to a Job's name in the workspace.
     * @return The jobId for the next job to be used for this Project.
     */
    QString reserveJobId();

    long getJobCount() const { return jobCount; }

private:
    QStringList sourceFileList;
    QString executablePath, outputDirPath, makeFilePath, name;

    long jobCount;
};

#endif // PROJECT_H
