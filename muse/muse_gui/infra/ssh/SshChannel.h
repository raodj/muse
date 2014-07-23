#ifndef SSHCHANNEL_H
#define SSHCHANNEL_H

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

#include <QObject>
#include "SshSocket.h"
#include <libssh2.h>
#include <QTextEdit>

/**
 * @brief The SshChannel class A class that makes the use of libssh2 seamless
 * by providing an API that uses one method to call on the needed libssh2
 * methods to achieve the desired result. This class is primarily used as a
 * helper class to RemoteServerSession to keep its code clean. As a result,
 * this class can also be used with the RSSAsyncHelper.
 */
class SshChannel : public QObject {
    Q_OBJECT
public:
    /**
     * @brief SshChannel Creates an SshChannel with socket's Libssh2_session
     * being used as this SshChannel's session instance variable. Needless to
     * say, the socket must not be NULL when this class is instantiated.
     * @param socket The SshSocket whose LIBSSH2_SESSION will be used with
     * this SshChannel.
     */
    SshChannel(SshSocket& socket);
    ~SshChannel();

    /**
     * @brief Executes to run a <b>brief</b> command that produces succint output.
     * The two possible outputs (standard output and error output) will be stored separately as Strings.
     *
     * @param command The command to be executed on the target machine.
     * The command must be compatible with the target machine's OS,
     * otherwise an exception will be generated.
     *
     * @param stdoutput The output that comes from the standard output
     * stream as a result of running the command.
     *
     *  @param stderrmsgs The output that comes from the standard error
     * stream as a result of running the command.
     *
     *  @return The exit code from the command that was run on the target machine.
     */
    int exec(const QString &command, QString &stdoutput,
             QString &stderrmsgs);

    /**
     * @brief exec Method to be run with a long running command that may
     * produce verbose output. The output is deposited in a QTextDocument
     * with appropriate styling given to the output returned from the command.
     * In other words, the standard output will appear differently than the
     * error stream and any other type of output.
     *
     * @param command The command to be executed on the target machine.
     * The command must be compatible with the target machine's OS,
     * otherwise an exception will be generated.
     *
     * @param output The QTextEdit that the output will be directed to and deposited in.
     *  This parameter cannot be null.
     *
     * @return The exit code from the command that was run on the target machine.
     */
    int exec(const QString& command, QTextEdit& output);

    /**
     * @brief copy A method to copy given data from an input stream to a given file on the server.
     * @param srcData The source stream that provides the data to be copied.
     *
     * @param destDirectory The destination directory to which the data is to be copied.
     * This method will assume the directory has already been created.
     *
     * @param destFileName The name of the destination file to which the data will be copied to.
     *
     * @param mode The POSIX compliant mode string (such as: "0600" or "0700") to be used
     * as the mode for the target file.
     */
    bool copy(const QString& srcDir, const QString &destDirectory,
               const QString &destFileName, const int& mode);

    /**
     * @brief copy Copy file from a remote machine to a given output stream.
     * @param destData The destination stream to which the data is to be written.
     * @param srcDirectory The source directory from where the file is to be copied.
     * @param srcFileName The name of the source file from where the data is to be copied.
     */
    bool copy(const QString& destData, const QString &srcDirectory,
               const QString &srcFileName);

private:
    LIBSSH2_SESSION* session;
    LIBSSH2_CHANNEL* channel;
};

#endif
