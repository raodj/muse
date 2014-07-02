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

#include <QApplication>
#include "SshException.h"
#include <exception>

template<typename RetVal>
RSSAsyncHelper<RetVal>::RSSAsyncHelper() {
}

template<typename RetVal>
RSSAsyncHelper<RetVal>::RSSAsyncHelper(RetVal* val, SshSocket* socket,
                                       MethodCall method,
                                       SFtpChannel* channel) : method(method) {
    result = val;
    this->socket = socket;
    sftpChannel = channel;
}

template<typename RetVal>
void
RSSAsyncHelper<RetVal>::run() {
    try {
        *result = method();
    }
    catch (const SshException &e) {
        const QString exceptionDetails =
                e.getErrorDetails().arg(e.getFileName(), QString::number(e.getLineNumber()),
                                 e.getMethodName(), QString::number(e.getSshErrorCode()),
                                 QString::number(e.getNetworkErrorCode()));
        emit exceptionThrown(e.getMessage(), e.getGenericErrorMessage(),
                             exceptionDetails);
    }
    catch (const std::exception& e ) {
        emit exceptionThrown(e.what());
    }

    result = NULL;
    returnLibSsh2ToMainThread();
}

template<typename RetVal>
void
RSSAsyncHelper<RetVal>::returnLibSsh2ToMainThread() {
    socket->changeToThread(QApplication::instance()->thread());
    socket = NULL;
    delete socket;
    if (sftpChannel != NULL) {
        sftpChannel->moveToThread(QApplication::instance()->thread());
        sftpChannel = NULL;
    }
    delete sftpChannel;
}

