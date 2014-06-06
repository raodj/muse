#ifndef MAIN_CPP
#define MAIN_CPP

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

#include "MainWindow.h"
#include "Version.h"
#include "ProgrammerLog.h"
#include <QApplication>
#include <QDebug>
#include <QFileSystemModel>
#include <QSortFilterProxyModel>
#include "FirstRunWizard.h"
#include <QDir>
#include <QFile>
#include "MUSEApplicationDirectory.h"

//testing
//#include "CustomFileDialog.h"
//#include <QFileDialog>

int main(int argc, char *argv[]) {
    // Setup custom logger to cut logs in programmer log
    // qInstallMessageHandler(Logger::cutLogEntry);
    // Initialize QT application (initally processes command-line arguments)
    QApplication app(argc, argv);
    app.setApplicationName("MUSE");
    // Create the main window that will contain various tabbed views that
    // can be reorganized by dragging and dropping tabs.
    MainWindow mainWindow;



    //Check that the application directory exists
    QDir workspaceDir(MUSEApplicationDirectory::getAppDirPath());
    if (!workspaceDir.exists()) {
        FirstRunWizard frw;
        frw.exec();//Don't show anything else until the user completes the dialog
    }

    else {
        //If it does exist, verify that known hosts file is present
        QFile knownHosts(MUSEApplicationDirectory::getKnownHostsPath());
        if(!knownHosts.exists()) {
            knownHosts.open(QFile::ReadWrite);
        }
        knownHosts.close();
    }


    mainWindow.show();
//    CustomFileDialog cfd;
//    cfd.show();
    // Start the main GUI-event processing loop.
    return app.exec();
}

#endif
