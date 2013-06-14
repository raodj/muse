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
#include "Logger.h"
#include "Version.h"
#include <QLabel>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    this->setWindowTitle("MUSE GUI");
    // Create the main desktop pane
    desktop = new DnDTabWidget(parent, "Desktop", true);
    // Set the desktop as the central widget
    this->setCentralWidget(desktop);
    // Add a couple of dummy tabs to the desktop for testing?
    // desktop->createSplitPane(new QLabel("Testing!"), "Testing", DnDTabBar::CENTER);
    // desktop->createSplitPane(new QLabel("Second"), "Second", DnDTabBar::CENTER);
    // Create default user and programmer log views
    DnDTabWidget* const bottomTab =
            desktop->createSplitPane(new UserLogView(), "User Log", DnDTabBar::BOTTOM,
                                     QIcon(":/images/32x32/user_logs.png"));
    bottomTab->createSplitPane(new ProgrammerLogView(),
                               "Programmer Log", DnDTabBar::CENTER,
                               QIcon(":/images/32x32/programmer_logs.png"));
}

MainWindow::~MainWindow() {
    // Nothing to be done here for now.
}

void
MainWindow::showEvent(QShowEvent *event) {
    Q_UNUSED(event);
    // Cut some initial programmer logs with disclaimers.
    progLog() << endl << QString(80, '-') << endl
              << FULL_TITLE << endl << MUSE_GUI_VERSION
              << MUSE_GUI_RELEASE_DATE << endl << MUSE_GUI_COPYRIGHT
              << endl << QString(80, '-');

    // Cut some initial user logs.
    userLog(Logger::NOTICE) << FULL_TITLE << endl << MUSE_GUI_VERSION;
    userLog(Logger::NOTICE) << MUSE_GUI_RELEASE_DATE << endl << MUSE_GUI_COPYRIGHT;
}

#endif
