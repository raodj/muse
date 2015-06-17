#ifndef MUSEAPPLICATIONDIRECTORY_CPP
#define MUSEAPPLICATIONDIRECTORY_CPP

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

#include "MUSEGUIApplication.h"
#include "FirstRunWizard.h"
#include "Workspace.h"
#include "Logger.h"
#include "workspaceWizard/WorkspaceWizard.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDialog>
#include <QMessageBox>

#include <vector>

//// Error message reported to user
//const QString MUSEGUIApplication::AppDirCreateErrMsg =
//        "Unable to create top-level application directory:\n%1\n" \
//        "MUSE (GUI) cannot proceed further without the top-level\n" \
//        "application directory. This is an unusual situation that\n" \
//        "may require your system administrator involvement to resolve.";

//// Error message reported to user
//const QString MUSEGUIApplication::KnownHostsCreateErrMsg =
//        "Unable to create the known hosts file required for SSH:\n%1\n" \
//        "MUSE (GUI) cannot proceed further without this default\n" \
//        "file required for operations. This is an unusual situation that\n" \
//        "may require your system administrator involvement to resolve.";

const QString MUSEGUIApplication::errorMessage =
        "There was a serious error during startup:\n%1\n" \
        "MUSE (GUI) cannot proceed further because of this error." \
        "This is an unusual situation that may require your system" \
        "administrator involvement to resolve.";

const QString MUSEGUIApplication::knownHostsFileName = "known_hosts";
const QString MUSEGUIApplication::workspacesFileName = "workspaces";

MUSEGUIApplication::MUSEGUIApplication(int &argc, char *argv[])
    : QApplication (argc, argv) {
    setApplicationName("MUSE");
}

MUSEGUIApplication::~MUSEGUIApplication() {

}

int
MUSEGUIApplication::exec() {
    if (testFirstRun() == QDialog::Rejected) {
        return 1;
    }

    createApplicationFiles();

    // before continuing, ask the user what workspace they want to use
    WorkspaceWizard ww(getWorkspacePaths());

    if (ww.exec() == QDialog::Rejected) {
        return 1;
    }

//    try {
//        muse::workspace::init();
//    } catch (QString err) {
//        QMessageBox::critical(NULL, "Error duing startup", errorMessage.arg(err));
//        return 2;
//    }

    // Everything went well so far. Create one main window and show it.
    mainWindow = new MainWindow();
    mainWindow->show();

    // Let the base class do rest of the work.
    return QGuiApplication::exec();
}

int
MUSEGUIApplication::testFirstRun() {
    QFileInfo knownHostsFile(knownHostsFilePath());
    QFileInfo workspacesFile(workspacesFilePath());

    if (!knownHostsFile.exists() || !workspacesFile.exists()) {
        QFile file(":/resources/welcome.html");
        FirstRunWizard frw(file);

        return frw.exec();
    }

    return QDialog::Accepted;
}

QString
MUSEGUIApplication::appDir() {
    return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
}

QString
MUSEGUIApplication::knownHostsFilePath() {
    return appDir() + QDir::separator() + knownHostsFileName;
}

QString
MUSEGUIApplication::workspacesFilePath() {
    return appDir() + QDir::separator() + workspacesFileName;
}

std::vector<QString>
MUSEGUIApplication::getWorkspacePaths() {
    std::vector<QString> ret;

    QFile in(workspacesFilePath());
    in.open(QIODevice::ReadOnly | QIODevice::Text);

    while (!in.atEnd()) {
        ret.push_back(QString(in.readLine()).trimmed());
    }

    return ret;
}

void
MUSEGUIApplication::addWorkspaceEntry(QString dir) {
    QFile out(MUSEGUIApplication::workspacesFilePath());
    out.open(QIODevice::Append | QIODevice::Text);

    QTextStream stream(&out);

    stream << dir;
}

void
MUSEGUIApplication::createApplicationFiles() {
    QFile knownHostsFile(MUSEGUIApplication::knownHostsFilePath());
    QFile workspacesFile(MUSEGUIApplication::workspacesFilePath());

    if (!knownHostsFile.exists() && !knownHostsFile.open(QFile::WriteOnly)) {
        throw QString("Failed to create known hosts file");
    }

    if (!workspacesFile.exists() && !workspacesFile.open(QFile::WriteOnly)) {
        throw QString("Failed to create workspaces file");
    }
}

#endif
