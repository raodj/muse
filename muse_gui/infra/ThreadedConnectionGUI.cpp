#ifndef THREADED_CONNECTION_GUI_CPP
#define THREADED_CONNECTION_GUI_CPP

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

#include <QMessageBox>
#include "ThreadedConnectionGUI.h"
#include "LoginCredentialsDialog.h"

QMutex ThreadedConnectionGUI::mutex;
QMutex ThreadedConnectionGUI::userDataMutex;

QWaitCondition ThreadedConnectionGUI::userHasResponded;
QWaitCondition ThreadedConnectionGUI::passUserData;

ThreadedConnectionGUI::ThreadedConnectionGUI() :
    QObject() {
}

ThreadedConnectionGUI::ThreadedConnectionGUI(const Server& server) :
    QObject(), server{ server } {
}

void
ThreadedConnectionGUI::interceptRequestForCredentials(QString* username, QString* passWord) {
    LoginCredentialsDialog lcd;
    lcd.setUsername(server.getUserID());
    // If there is no password (most cases), this will be blank
    lcd.setPassword(server.getPassword());
    // Only show the dialog if the password is not - ie,
    // The ServerWizard is not running - we know the password already
    if (lcd.getPassword().isEmpty()) {
        // We don't know the password, so get it from the user.
        lcd.exec();
    }
    // Prevent other threads from accessing this data.
    userDataMutex.lock();
    // Change the username credential to the username input in the wizard
    *username = lcd.getUserName();
    // Change the password credential to the password input in the wizard
    *passWord = lcd.getPassword();
    // Save the password so we don't need to prompt again during the
    // life of the ServerSession
    server.setPassword(lcd.getPassword());
    // Let the background thread continue
    passUserData.wakeAll();
    // Release the lock on the data
    userDataMutex.unlock();
}

void
ThreadedConnectionGUI::promptUser(const QString& windowTitle,
                                   const QString& text,
                                   const QString& informativeText,
                                   const QString& detailedText,
                                   int *userChoice) {

    QMessageBox msgBox;
    msgBox.setWindowTitle(windowTitle);
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(text);
    msgBox.setInformativeText(informativeText);
    msgBox.setDetailedText(detailedText);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    mutex.lock();
    *userChoice = msgBox.exec();
    userHasResponded.wakeAll();
    mutex.unlock();
}

void
ThreadedConnectionGUI::showException(const QString &message,
                              const QString &genErrorMessage,
                              const QString &exceptionDetails) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("SSH connectivity error");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(message);
    msgBox.setInformativeText(genErrorMessage);
    msgBox.setDetailedText(exceptionDetails);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setDefaultButton(QMessageBox::Ok);
    msgBox.exec();
}

void
ThreadedConnectionGUI::showMessage(const QString &message) {
    QMessageBox msgBox;
    msgBox.setWindowTitle("Message");
    msgBox.setText(message);
    msgBox.exec();
}


#endif
