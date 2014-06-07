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
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QDialog>
#include <QMessageBox>

// Error message reported to user
const QString MUSEGUIApplication::AppDirCreateErrMsg =
        "Unable to create top-level application directory:\n%1\n" \
        "MUSE (GUI) cannot proceed further without the top-level\n" \
        "application directory. This is an unusual situation that\n" \
        "may require your system administrator involvement to resolve.";

// Error message reported to user
const QString MUSEGUIApplication::KnownHostsCreateErrMsg =
        "Unable to create the known hosts file required for SSH:\n%1\n" \
        "MUSE (GUI) cannot proceed further without this default\n" \
        "file required for operations. This is an unusual situation that\n" \
        "may require your system administrator involvement to resolve.";

MUSEGUIApplication::MUSEGUIApplication(int &argc, char *argv[])
    : QApplication (argc, argv) {
    setApplicationName("MUSE");
}

MUSEGUIApplication::~MUSEGUIApplication() {
    while (!mainWindowList.empty()) {
        delete mainWindowList[0];
        mainWindowList.removeFirst();
    }
}

int
MUSEGUIApplication::exec() {
    // Check that the application directory exists
    QDir workspaceDir(getAppDirPath());
    if (!workspaceDir.exists()) {
        // It is assumed this is the first time the user is running
        // MUSE GUI! Run the FirstRunWizard dialog.
        FirstRunWizard frw(*this);
        if (frw.exec() == QDialog::Rejected) {
            // The user rejected or canceled the start. Exit right away
            return 1;
        }
    }
    // Ensure other files look all right at this point in time.
    if (!checkCreateAppDirectory()) {
        // Error when setting up necessary files. Bail out.
        return 2;
    }

    // Everything went well so far. Create one main window and show it.
    mainWindowList.append(new MainWindow());
    mainWindowList[0]->show();
    // Let the base class do rest of the work.
    return QGuiApplication::exec();
}


QString
MUSEGUIApplication::getAppDirPath() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

QString
MUSEGUIApplication::getKnownHostsPath() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation)
            + "/known_hosts";
}


bool
MUSEGUIApplication::checkCreateAppDirectory(QWidget *parent) {
    // Check and create the top-level application directory as needed.
    QDir appDir(MUSEGUIApplication::getAppDirPath());

    if (!appDir.exists()) {
        if (!appDir.mkdir(MUSEGUIApplication::getAppDirPath())) {
            // Report error message and bail out
            QMessageBox::critical(parent, "Error during startup",
                                  AppDirCreateErrMsg.arg(getAppDirPath()));
            return false;
        }
    }
    // Check and create the known hosts file as needed.
    QFile knownHosts(MUSEGUIApplication::getKnownHostsPath());
    if (!knownHosts.exists()) {
        if (!knownHosts.open(QFile::WriteOnly)) {
            // Report error message and bail out with failure
            QMessageBox::critical(parent, "Error during startup",
                                  KnownHostsCreateErrMsg.arg(getKnownHostsPath()));
            return false;
        }
    }
    // Everything seems in order
    return true;
}

#endif
