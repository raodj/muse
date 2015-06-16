#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

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

#include "DnDTabWidget.h"

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>

#include <memory>

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget *parent = 0);

    ~MainWindow();

protected:
    void showEvent(QShowEvent *event);

protected slots:
//    void createLoadDefaultWorkspace();


    void showGeospatialView();

    void showProjectsJobsListView();

    /**
     * @brief showProjectWizard Creates and executes the ProjectWizard
     * when the newProject QAction is triggered.
     */
    void showProjectWizard();

    /**
     * @brief showJobWizard Creates and executes the JobWizard when the
     * newJob QAction is triggered.
     */
    void showJobWizard();

private:
    /**
     * @brief desktop The permanent desktop area for displaying core
     * information about a MUSE model/simulation. This desktop area
     * essentially holds tabs that can be opened/closed as needed.
     */
    //std::unique_ptr<DnDTabWidget> desktop;
    //std::unique_ptr<DnDTabWidget> bottomTab;
    //std::unique_ptr<QAction> newProject;
    //std::unique_ptr<QAction> newJob;

    //QMenu fileMenu;


    DnDTabWidget *desktop;
    QMenu fileMenu;
    QAction *newProject;
    QAction *newJob;

    /**
     * @brief showServerWidget Displays a server list view in the main frame
     * if a view is not already present.
     *
     * This is a convenience method to display the server list view in this
     * main frame. This method performs the necessary action only if a
     * server view is not already present.  If a server view is already present
     * then this method does not perform any operations.  This method may be
     * invoked via the top-level application's "View" menu option.
     */
    void showServerListView();
    void showProjectListView();
    void showJobListView();

    /**
     * @brief createMenus Creates the menu bar that is used throughout
     * MUSE_GUI.
     */
    void createMenus();

    /**
     * @brief createActions Creates the actions that are used in
     * MUSE_GUI's menu bar.
     */
    void createActions();

};

#endif // MAIN_WINDOW_H
