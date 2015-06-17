#ifndef MUSE_GUI_APPLICATION_H
#define MUSE_GUI_APPLICATION_H

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

#include <QString>
#include <QApplication>

#include <memory>

/**
 * @brief The MUSEGUIApplication class The main class for the MUSE GUI.
 *
 * This class handles and manages the execution of the entire GUI system.
 */
class MUSEGUIApplication : public QApplication {
public:
    /**
     * @brief MUSEGUIApplication The only constructor to create a top-level
     * application class.
     *
     * The constructor is relatively straightforward and merely passes
     * the parameters to the base class. The core operations of this
     * class are performed when the exec() method is invoked.
     *
     * @param argc The number of command-line arguments to the program.
     *
     * @param argv The command-line arguments to the program.
     */
    MUSEGUIApplication(int &argc, char *argv[]);

    /**
     * @brief ~MUSEGUIApplication The destructor.
     *
     * The destructor frees up all the memory allocated to hold the
     * main window.
     */
    virtual ~MUSEGUIApplication();

    /**
     * @brief exec The top-level method that kick starts various operations
     * in the GUI.
     *
     * This method is essentially a wrapper around call to
     * QGUIApplication::exec(). In addition, this method performs initial
     * checks to launch the FirstRunWizard (if the user is running MUSE GUI)
     * for the first time.
     *
     * @return This method returns the value from QGUIApplication::exec().
     */
     int exec();

     static QString appDir();

     static QString knownHostsFilePath();

     static QString workspacesFilePath();

     static std::vector<QString> getWorkspacePaths();

     static void addWorkspace(QString dir);

private:
     /**
     * @brief mainWindow The top-level windows that are used to display
     * information to the user. There is always at least one main window
     * for the application. However, the user may have multiple main
     * windows open to facilitate viewing data using multiple monitors
     * or simply for convenience.
     *
     * \note With Qt's object hierarchy (in which MainWindow is not
     * copyable), we are sadly forced to use pointers rather than objects.
     */
     MainWindow *mainWindow;
     //std::unique_ptr<MainWindow> window;

     static const QString errorMessage;

     static const QString knownHostsFileName;
     static const QString workspacesFileName;

     int testFirstRun();

     void createApplicationFiles();
};

#endif
