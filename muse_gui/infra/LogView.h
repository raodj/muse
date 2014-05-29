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
#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>
#include <qlineedit.h>
#include <qlabel.h>
#include <qaction.h>
#include <qtoolbar.h>
#include <qfiledialog.h>
#include <qcheckbox.h>
#include "Log.h"
/**
 * @brief A base class for the log views at the bottom of the main window.
 * Initially, this class is mainly responsible for the creation of a
 * toolbar that will be displayed with the logs that will allow the user to
 * save the logs to a file.
 */
class LogView : public QWidget{
    Q_OBJECT
public:

    /**
     * @brief A simple constructor for the base LogView class. Other than
     * storing the log pointer to its instance variable, the constructor
     * creates the toolbar for the log view tabs.
     * @param parent The parent QWidget of this LogView.
     * @param log The log that is shown when this toolbar is dislpayed on the
     * screen.
     */
    LogView(QWidget *parent = 0);

    /**
     * @brief Updates the log entry with the newest information. This is an abstract
     * method to be overriden in the child classes.
     */
    virtual void updateLog() = 0;

signals:
    /**
     * @brief Alerts the program that the name of the file to which the user
     * has selected to save the log entries has changed.
     *
     * @param filePath The new file path of the file that will contain the log
     * reports.
     */
    void logFileNameChanged(const QString &filePath);

    void saveFileNow();

public slots:

    /**
     * @brief Updates the displayed file name of the log file by using the log
     * class' getLogFileName().
     */
    virtual void updateFileName() = 0;

    /**
     * @brief Triggers a save dialog box to select where the log data
     * will be saved.
     */
    void selectNewLogFile();

    /**
     * @brief Changes the shouldSave boolean and the icon of the
     * saveToggleButton to reflect the user's decision on whether
     * or not to save the log file.
     */
    void updateSavePreference();

protected:
    //Log *log;
    QLabel *fileNameLabel;
    QLineEdit *fileNameDisplay;
    QAction *changeLogFileName;
    QAction *saveToggleButton;
    QToolBar *logToolBar;
    bool shouldSave;
private:
    /**
     * @brief Creates the QLabel for the toolbar.
     */
    void initializeLabel();

    /**
     * @brief Creates the QLineEdit that displays the current location
     * of the log save file.
     */
    void initializeFileNameDisplay();

    /**
     * @brief Creates the base QActions associated with the toolbar,
     * namely, the action to change the log save file and the start and
     * stop saving action.
     */
    void initializeActions();

    /**
     * @brief Initializes the toolbar and puts all of its pieces together.
     */
    void createToolBar();
};

#endif // LOGVIEW_H
