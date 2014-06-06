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

MUSEGUIApplication::MUSEGUIApplication(int &argc, char *argv[])
    : QApplication (argc, argv) {
    this->setApplicationName("MUSE");

    int frwResult = QDialog::Accepted;
    //Check that the application directory exists
    QDir workspaceDir(getAppDirPath());
    if (!workspaceDir.exists()) {
        FirstRunWizard frw;
        frwResult = frw.exec();
    }

    else {
        //If it does exist, verify that known hosts file is present
        QFile knownHosts(getKnownHostsPath());
        if(!knownHosts.exists()) {
            knownHosts.open(QFile::ReadWrite);
        }
        knownHosts.close();
    }

    //If the user skips the FirstRunWizard, don't make the main window
    if (frwResult != QDialog::Rejected) {
        // Create the main window that will contain various tabbed views that
        // can be reorganized by dragging and dropping tabs.
        mainWindow = new MainWindow();

    }
    //    CustomFileDialog cfd;
    //    cfd.show();
}

/*int
MUSEGUIApplication::exec() {
    else  return 0;

    //return 0;
}*/


QString
MUSEGUIApplication::getAppDirPath() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation);
}

QString
MUSEGUIApplication::getKnownHostsPath() {
    return QStandardPaths::writableLocation(QStandardPaths::DataLocation)
            + "/known_hosts";
}

#endif
