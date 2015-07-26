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
#include "WorkspaceDialog.h"
#include "ServerWatcher.h"

#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDialog>
#include <QMessageBox>

#include <vector>
#include <algorithm>

// The known hosts file name.
const QString MUSEGUIApplication::knownHostsFileName = "known_hosts";
// The file with list of workspaces.
const QString MUSEGUIApplication::workspacesFileName = "workspaces";

MUSEGUIApplication::MUSEGUIApplication(int &argc, char *argv[])
    : QApplication (argc, argv) {
    setApplicationName("MUSE");
}

MUSEGUIApplication::~MUSEGUIApplication() {
    // Nothing else to be done for now.
}

int
MUSEGUIApplication::exec() {
    // Check with user about licensing on first run on this machine.
    if (testFirstRun() == QDialog::Rejected) {
        return 1;
    }
    // Check and create application directory and work files.
    if (!createApplicationFiles()) {
        return 2;  // error with top-level files!
    }

    // Before continuing, ask the user what workspace they want to use
    WorkspaceDialog wd(getWorkspacePaths());
    if (wd.exec() == QDialog::Rejected) {
        return 1;
    }

    // Our workspace has been set up and the gui is ready to be displayed,
    // so lets start checking the servers for updates
    serverWatcher.start();

    // Everything went well so far. Create one main window and show it.
    mainWindow = new MainWindow();
    mainWindow->show();

    // Let the base class do rest of the work.
    return QGuiApplication::exec();
}

int
MUSEGUIApplication::testFirstRun() {
    // We need to have these two files to proceed further.
    QFileInfo knownHostsFile(knownHostsFilePath());
    QFileInfo workspacesFile(workspacesFilePath());

    if (!knownHostsFile.exists() || !workspacesFile.exists()) {
        // Looks like the first run on this machine.
        FirstRunWizard frw(*this);
        return frw.exec();
    }

    // Everything is fine so far and it is not the first run.
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

QString
MUSEGUIApplication::getUserName() {
    // Gets the username on the system. "USER" for linux/Mac,
    // "USERNAME" for Windows
    QString userName = qgetenv("USER");

    if (userName.isEmpty()) {
        userName = qgetenv("USERNAME");
    }

    return userName;
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
    std::vector<QString> currentEntries = getWorkspacePaths();

    if (std::find(std::begin(currentEntries), std::end(currentEntries),
                  dir) != std::end(currentEntries)) {
        return;
    }

    QFile out(MUSEGUIApplication::workspacesFilePath());
    out.open(QIODevice::Append | QIODevice::Text);

    QTextStream stream(&out);

    stream << dir << endl;
}

bool
MUSEGUIApplication::createApplicationFiles(QWidget* parent) {
    QString errMsg;  // Modified below to indicate error conditions.

    // First check and create the application directory. This is
    // absolutely needed the directory exists first in Linux & Windows.
    QDir mainDir(appDir());
    if (!mainDir.exists() && !mainDir.mkdir(appDir())) {
        errMsg = "Unable to create top-level application directory: " +
                appDir();
    } else {
        // Check and create the known hosts file
        QFile knownHostsFile(MUSEGUIApplication::knownHostsFilePath());
        if (!knownHostsFile.exists() && !knownHostsFile.open(QFile::WriteOnly)) {
            errMsg = "Unable to create required KnownHosts file at: " +
                    knownHostsFile.fileName();
        }
        // Check and create the workspaces file.
        QFile workspacesFile(MUSEGUIApplication::workspacesFilePath());
        if (errMsg.isEmpty() && !workspacesFile.exists() &&
            !workspacesFile.open(QFile::WriteOnly)) {
            errMsg = "Unable to create required workspaces file at: " +
                    workspacesFile.fileName();
        }
    }
    // Check and report any errors that may have occurred
    if (!errMsg.isEmpty()) {
        // Error occurred!
        errMsg += "\n\nMUSE (GUI) cannot proceed further without the top-level\n"  \
            "application directory or file(s). This is an unusual situation that\n"\
            "may require your system administrator involvement to resolve.";
        QMessageBox::critical(parent, "Startup error", errMsg);
        return false; // error
    }
    return true; // success
}

#endif
