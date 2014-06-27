#ifndef RSSASYNCHELPER_H
#define RSSASYNCHELPER_H

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
#include "SshSocket.h"
#include <functional>

template<typename RetVal>
/**
 * @brief The RSSAsyncHelper class A helper class to perform
 * operations that would be possibly time consuming or operations
 * that could otherwise lock the user interface.
 */
class RSSAsyncHelper : public QThread {
typedef std::function<RetVal(void)> MethodCall;
public:

    RSSAsyncHelper();
    /**
     * @brief RSSAsyncHelper Creates an instance of the RSSAsyncHelper.
     * @param val A pointer to the location where the return result of
     * method will be stored. A pointer is used here since a template class
     * (RSSAsyncHelper) cannot send signals, other than finished(). When
     * finished is emitted, it is safe to check the pointer for feedback on
     * the operation being called in run().
     * @param socket The SshSocket that will be used as part of the method
     * being called in run().
     * @param method The method to call, passed to the constructor through
     * the use of std::bind.
     * @param channel The SFtpChannel to be used as part of the method
     * being called in run, which, depending on the method, may not be required,
     * hence the default value for it is null.
     */
    RSSAsyncHelper(RetVal* val, SshSocket* socket, MethodCall method, SFtpChannel* channel = NULL);

    /**
     * @brief run Calls the method passed to this RSSAsyncHelper in the
     * constructor. Once the task is complete, it calls returnSocketToMainThread().
     */
    void run();

private:
    RetVal* result;
    MethodCall method;
    SshSocket* socket;
    SFtpChannel* sftpChannel;

    /**
     * @brief returnLibSsh2ToMainThread Returns thread ownership of
     * the SshSocket and SFtpChannel (if applicable) back to the
     * Qt Application Thread.
     */
    void returnLibSsh2ToMainThread();

};

#include "RSSAsyncHelper.ipp"

#endif // RSSASYNCHELPER_H
