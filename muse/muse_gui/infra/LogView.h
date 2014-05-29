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
#include <QLabel>
#include <qtoolbar.h>
#include "Log.h"
/**
 * @brief A base class for the log views at the bottom of the main window.
 * Initially, this class is mainly responsible for the creation of a
 * toolbar that will be displayed with the logs that will allow the user to
 * save the logs to a file.
 */
class LogView : public QWidget{
    friend class Log;
    Q_OBJECT
public:
    LogView(QWidget *parent = 0, Log *log = NULL);
    virtual void updateLog() = 0;

signals:

public slots:
    void updateFileName();

protected:
    Log *log;
    QLabel *fileNameLabel;
    QLineEdit *fileNameDisplay;
    QAction *changeLogFileName;
    QAction *saveToggleButton;
    QToolBar *logToolBar;

private:
    void initializeLabel();
    void initializeFileNameDisplay();
    void initializeActions();
    void createToolBar();
};

#endif // LOGVIEW_H
