#ifndef FSM_ASYNC_HELPER_H
#define FSM_ASYNC_HELPER_H
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

#include <QRunnable>
#include <QObject>
#include <QModelIndex>

#include "FSHelper.h"

/**
 * @brief The FSMAsyncHelper class a threaded class that assists
 * in the loading of files from the remote server in an
 * asynchronus matter, meaning that the GUI will not be locked
 * while this process is executing.
 */
class FSMAsyncHelper : public QObject, public QRunnable {
    Q_OBJECT
public:
    FSMAsyncHelper(const QModelIndex &parent,
                   FSHelper* helper, const FSEntry& dir);

    /**
     * @brief run The threaded method that begins loading the
     * directories requested by the user in the gui. Once complete,
     * the method emits the entriesLoaded() signal.
     */
    void run();

signals:
    void entriesLoaded(const QModelIndex& modelIndex);

private:
    const QModelIndex modelIndex;
    FSHelper* const fsHelper;
    const FSEntry& dir;
};

#endif // FSM_ASYNC_HELPER_H
