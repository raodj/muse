#ifndef MUSETHREAD_H
#define MUSETHREAD_H

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

#include <QThread>

/**
 * @brief The MUSEThread class A base class for thread classes that
 * are used in MUSE. Primarily, this class exists so that the template
 * thread class, RSSAsyncHelper, can emit signals to the main thread
 * that an exception was thrown and that the user should be notified of
 * the exception. There should never be a variable of type MUSEThread,
 * but instead one of its child classes.
 */
class MUSEThread : public QThread {
    Q_OBJECT
public:
    MUSEThread();

signals:
    /**
     * @brief exceptionThrown Alerts the main Qt gui thread that an
     * exception was thrown and that the program needs to display a
     * warning to the user explaining what happened.
     * @param message The SshException message.
     * @param genErrorMessage The SshException general message.
     * @param exceptionDetails The details of the SshException thrown
     */
    void exceptionThrown(const QString& message, const QString& genErrorMessage,
                         const QString& exceptionDetails);
    /**
     * @brief exceptionThrown Alerts the main Qt gui thread that an
     * exception was thrown and that the program needs to display a
     * warning to the user explaining what happened.
     * @param message The exception thrown.
     */
    void exceptionThrown(const QString& message);
};

#endif // MUSETHREAD_H
