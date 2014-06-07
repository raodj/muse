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

#include <QString>
#include <QApplication>
#include "MainWindow.h"
#include <QList>

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
     * @brief getWorkSpacePath Gets the file path to the application
     * directory.
     * @return A QString representing the file path to the application
     * directory.
     */
    static QString getAppDirPath();

    /**
     * @brief getKnownHostsPath Gets the file path to the known_hosts
     * file within the application directory.
     * @return A QString representing the file path to the known_hosts file.
     */
     static QString getKnownHostsPath();

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
     * @brief checkCreateApplicationDirectory Convenience method to check
     * and create top-level, user-specific application directory.
     *
     * \note Calling this method checks and creates files only as needed.
     * Consequently, it is always safe to call this method and if files
     * exist, this method returns true without any other side effects.
     *
     * This method is used to check and create the top-level application
     * directory along with some of the default files that are used
     * by MUSE GUI. Note that there is exactly only one application
     * directory for each user.  Do not confuse the application directory
     * with the workspace directory (a user can have many workspace
     * direcotires). This method checks and creates the following
     * directories:
     *
     * <ol>
     *
     * <li>The top-level application directory returned by
     * MUSEGUIApplication::getAppDirPath</li>
     *
     * <li>The \c known_hosts file (used by SSH connection) in the
     * top-level directory</li>
     *
     * </ol>
     *
     * @param parent An optional parent window to be used for reporting
     * error messages, if any errors occurr.
     *
     * @return This method returns true if the application structure looks
     * good and the basic set of files are present. If the files could not
     * be created then this method returns false. See earlier note about
     * safety of this method.
     */
     bool checkCreateAppDirectory(QWidget *parent = 0);

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
     QList<MainWindow*> mainWindowList;

     /**
      * @brief AppDirCreateErrMsg Error message formatted and displayed
      * to the user when the top-level aplication directory could not be
      * created.
      */
     static const QString AppDirCreateErrMsg;

     /**
      * @brief KnownHostsCreateErrMsg Error message formatted and displayed
      * to the user when the known hosts file could not be
      * created.
      */
     static const QString KnownHostsCreateErrMsg;
};

#endif
