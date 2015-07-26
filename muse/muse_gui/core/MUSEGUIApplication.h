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
#include "ServerWatcher.h"

#include <QString>
#include <QApplication>

/**
 * @brief The MUSEGUIApplication class The main class for the MUSE GUI.
 *
 * This class handles and manages the execution of the entire GUI system.
 */
class MUSEGUIApplication : public QApplication {
    friend class FirstRunWizard;
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

     /**
      * @brief appDir The directory where application data for MUSE GUI will
      * be stored
      *
      * This is not the same place that information about workspaces will be
      * stored and this is not a user defined location.  For example, on linux
      * this will return /home/username/.local/share/MUSE
      *
      * @return A writable location where MUSE GUI data can be stored
      */
     static QString appDir();

     /**
      * @brief knownHostsFilePath The file location of the known hosts file
      * used by the SSH code
      *
      * This file is located in the directory returned by appDir().  It will
      * return /home/username/.local/share/MUSE/known_hosts on linux
      *
      * @return The absolute file path of the known_hosts file
      */
     static QString knownHostsFilePath();

     /**
      * @brief workspacesFilePath The file location of the knwon workspaces file
      *
      * This file is located in the directory returned by appDir().  It will
      * return /home/username/.local/share/MUSE/workspaces on linux
      *
      * @return The absolute file path of the workspaces file
      */
     static QString workspacesFilePath();

     static QString getUserName();

     /**
      * @brief getWorkspacePaths A list of every known workspace location
      *
      * Workspace directory locations are taken from the file at the location
      * returned by workspacesFilePath()
      *
      * @return A vector of QStrings containing every known workspace location
      */
     static std::vector<QString> getWorkspacePaths();

     /**
      * @brief addWorkspaceEntry Add a workspace to the known workspaces file
      *
      * This does not actually create a new workspace, it only adds the given
      * directory to the file at the location returned by workspacesFilePath()
      *
      * @param dir The directory location of the workspace to add
      */
     static void addWorkspaceEntry(QString dir);

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

     /**
      * @brief serverWatcher The class that will periodically check
      * all the servers in the current workspace for updates.  This class
      * will run in its own thread and will produce signals upon encountering
      * an error with a server which will be handled/displayed by the gui
      */
     ServerWatcher serverWatcher;

     /** This string contains the preferred name for the known hosts
      * file used by SSH to validate connections to remote servers.
      * Note that this string is just the file name without parent path.
      */
     static const QString knownHostsFileName;

     /** The path to the workspaces file name that contains the list of
      * known workspaces that the user can select from.
      */
     static const QString workspacesFileName;

     /**
      * @brief testFirstRun Test if this is the first time MUSE GUI has been
      * run by this user
      *
      * Will launch the FirstRunWizard if this is actually the first time the
      * GUI has been run
      *
      * @return The return code of the FirstRunWizard
      */
     int testFirstRun();

     /**
      * @brief createApplicationFiles Create the necessary application files
      * if they dont exist
      *
      * creates the files at the locations indicated by knownHostsFilePath() and
      * workspacesFilePath() and reports any errors that occur.
      *
      * @param parent The parent to be used when displaying error dialog boxes
      *
      * @return This method returns true if the top-level files were
      * successfully created or were already present.
      */
     bool createApplicationFiles(QWidget *parent = 0);
};

#endif
