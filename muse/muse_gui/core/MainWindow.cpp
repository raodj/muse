#ifndef MAIN_WINDOW_CPP
#define MAIN_WINDOW_CPP

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

#include "Core.h"
#include "MainWindow.h"
#include "ProgrammerLogView.h"
#include "UserLogView.h"
#include "Workspace.h"
#include "Logger.h"
#include "Version.h"
#include <QLabel>
#include <QVBoxLayout>
#include <QDir>
#include <QTimer>
#include <QShowEvent>
#include "MUSEGUIApplication.h"

//Testing
#include "ServerListView.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setWindowTitle("MUSE GUI");
    // Create the main desktop pane
    desktop = new DnDTabWidget(parent, "Desktop", true);
    // Set the desktop as the central widget
    this->setCentralWidget(desktop);
    // Create default user and programmer log views
    DnDTabWidget* const bottomTab =
            desktop->createSplitPane(new UserLogView(), "User Log",
                                     DnDTabBar::BOTTOM,
                                     QIcon(":/images/32x32/user_logs.png"));
    bottomTab->createSplitPane(new ProgrammerLogView(),
                               "Programmer Log", DnDTabBar::CENTER,
                               QIcon(":/images/32x32/programmer_logs.png"));

    buildServerWidget();

    addDockWidget(Qt::LeftDockWidgetArea, serverWidget, Qt::Vertical);


}

MainWindow::~MainWindow() {
    // Nothing to be done here for now.
}

void
MainWindow::showEvent(QShowEvent *event) {
    // Let base class do the necessary operations
    QMainWindow::showEvent(event);
    if (event->spontaneous()) {
        return;
    }
    // Cut some initial programmer logs with disclaimers.
    progLog() << endl << QString(80, '-') << endl
              << FULL_TITLE << endl << MUSE_GUI_VERSION
              << MUSE_GUI_RELEASE_DATE << endl << MUSE_GUI_COPYRIGHT
              << endl << QString(80, '-');

    // Cut some initial user logs.
    userLog() << FULL_TITLE << endl << MUSE_GUI_VERSION;
    userLog() << MUSE_GUI_RELEASE_DATE << endl << MUSE_GUI_COPYRIGHT;
    // Setup signal to create/load workspace. This should eventually become
    // a dialog that permits the user to choose a workspace even before
    // the main window is launched.
    QTimer::singleShot(50, this, SLOT(createLoadDefaultWorkspace()));
}

void
MainWindow::createLoadDefaultWorkspace() {
    // Try and load the default workspace first.
    const QString homeDir = MUSEGUIApplication::getAppDirPath();
    QString errMsg = Workspace::useWorkspace(homeDir);
    if (errMsg != "") {
        userLog(Logger::LOG_WARNING)
                << "Default workspace file was not found in " << homeDir << " [" << errMsg << "]";
        userLog(Logger::LOG_WARNING)
                << "Attempting to create default workspace in " << homeDir;
        if ((errMsg = Workspace::createWorkspace(homeDir)) == "") {
            userLog() << "Successfully created workspace in " << homeDir;
        }
    }
    if (errMsg == "") {
        userLog() << "Using workspace in directory " << homeDir;
    } else {
        userLog(Logger::LOG_ERROR) << "Error creating/using workspace in "
                                   << homeDir << " - " << errMsg;
    }
}

void
MainWindow::buildServerWidget() {
    //Create the widget and set its minimum width.
    serverWidget = new QDockWidget("Servers");
    serverWidget->setMinimumWidth(this->width() / 5);

//    QVBoxLayout* mainLayout = new QVBoxLayout();
//    mainLayout->addWidget(new ServerListView());

    //Apply the view as the main widget for the view.
    serverWidget->setWidget(new ServerListView());
}

#endif
