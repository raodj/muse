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

#include <QLabel>
#include <QVBoxLayout>
#include <QDir>
#include <QTimer>
#include <QShowEvent>
#include <QMenuBar>
#include <QMessageBox>

#include "MainWindow.h"
#include "Core.h"
#include "ProgrammerLogView.h"
#include "UserLogView.h"
#include "Logger.h"
#include "Version.h"
#include "ProjectWizard.h"
#include "JobWizard.h"
#include "ServerListView.h"
#include "GeospatialView.h"
#include "Workspace.h"
#include "WorkspaceDialog.h"
#include "MUSEGUIApplication.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setWindowTitle("MUSE GUI");

    // Create the main desktop pane
    desktop = new DnDTabWidget(parent, "Desktop", true);

    // Set the desktop as the central widget
    this->setCentralWidget(desktop);

    // Create default user and programmer log views
    DnDTabWidget *bottomTab = desktop->createSplitPane(new UserLogView(), "User Log",
                                         DnDTabBar::BOTTOM,
                                         QIcon(":/images/32x32/user_logs.png"));

    bottomTab->createSplitPane(new ProgrammerLogView(),
                               "Programmer Log", DnDTabBar::CENTER,
                               QIcon(":/images/32x32/programmer_logs.png"));

    createActions();
    createMenus();
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

    showServerListView();
    showProjectListView();
    showJobListView();

    showGeospatialView();
}

void
MainWindow::showServerListView() {
    // Check to see if a server list view already exists in this main window.
    // If so do not perfrom any futher operations.
    if (findChild<ServerListView*>(ServerListView::ViewName) == NULL) {
        // Create the widget and set its minimum width.
        ServerListTableModel *model = new ServerListTableModel(Workspace::get()->getServerList());
        desktop->createSplitPane(new ServerListView(model), "Servers",
                                 DnDTabBar::LEFT,
                                 QIcon(":/images/16x16/Server.png"));
    }
}

void
MainWindow::showGeospatialView() {
    // Check to see if a geospatial view already exists in this main window.
    // If so do not perfrom any futher operations.
    if (findChild<GeospatialView*>(GeospatialView::ViewName) == NULL) {
        // Create the widget and set its minimum width.
        desktop->createSplitPane(new GeospatialView(), "Geospatial",
                                 DnDTabBar::LEFT,
                                 QIcon(":/images/16x16/Server.png"));
    }
}

void
MainWindow::showProjectListView() {
    // Check to see if a projects list view already exists in this main window.
    // If so do not perfrom any futher operations.
    /*
    if (findChild<ProjectListView*>(ProjectListView::ViewName) == NULL) {
        // Create the widget and set its minimum width.
        desktop->createSplitPane(new ProjectListView(), "Projects",
                                 DnDTabBar::CENTER,
                                 QIcon(":/images/16x16/Server.png"));
    }
    */
}

void
MainWindow::showJobListView() {
    // will be used to show jobs view when it is created
    /*
    if (findChild<JobListView*>(JobListView::ViewName) == NULL) {
        // Create the widget and set its minimum width.
        desktop->createSplitPane(new JobListView(), "Jobs",
                                 DnDTabBar::CENTER,
                                 QIcon(":/images/16x16/Server.png"));
    }
    */
}


void
MainWindow::showProjectWizard() {
    QFile file (":/resources/projectOverview.html");
    ProjectWizard prgWiz(file);
    prgWiz.exec();
}

void
MainWindow::showJobWizard() {
    QFile file(":/resources/jobOverview.html");
    JobWizard jobWiz(file);
    jobWiz.exec();
}

void
MainWindow::showWorkspaceWizard() {
    //WorkspaceWizard wiz(MUSEGUIApplication::getWorkspacePaths());
    WorkspaceDialog dialog(MUSEGUIApplication::getWorkspacePaths());
    dialog.exec();
}

void
MainWindow::quitMUSE() {
    Workspace *ws = Workspace::get();

    // save the current workspace before we quit, just to make sure every change
    // the user made gets saved
    QString error = ws->saveWorkspace();

    // if there was a problem with saving, let the user know, and ask them if
    // they want to continue exiting MUSE
    if (error != "") {
        QMessageBox::StandardButton reply =
                QMessageBox::question(this, "Error",
                                      "There was a problem saving the workspace:"
                                      + error + ".  Do you still wish to exit MUSE?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return;
        }
    }

    // exit MUSE with a return code of 0
    QApplication::exit();
}

void
MainWindow::createActions() {
    newProject = new QAction("Create New Project", this);
    connect(newProject, SIGNAL(triggered()), this, SLOT(showProjectWizard()));

    newJob = new QAction("Create New Job", this);
    connect(newJob, SIGNAL(triggered()), this, SLOT(showJobWizard()));

    switchWorkspace = new QAction("Switch Workspace", this);
    connect(switchWorkspace, SIGNAL(triggered()), this, SLOT(showWorkspaceWizard()));

    quit = new QAction("Exit MUSE", this);
    connect(quit, SIGNAL(triggered()), this, SLOT(quitMUSE()));
}

void
MainWindow::createMenus() {
    fileMenu.setTitle("File");
    fileMenu.addAction(newProject);
    fileMenu.addAction(newJob);
    fileMenu.addSeparator();
    fileMenu.addAction(switchWorkspace);
    fileMenu.addSeparator();
    fileMenu.addAction(quit);

    menuBar()->addMenu(&fileMenu);
}

#endif
