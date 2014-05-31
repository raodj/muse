#ifndef LOG_H
#define LOG_H

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

#include "Logger.h"
#include <QFile>
#include <QTextStream>

class Log : public QObject {
    Q_OBJECT
public:
    virtual bool isUserLog() const { return false; }

    /**
     * @brief Gets the name of the log file.
     * @return The name of the log file.
     */
    QString getLogFileName();



    /**
     * @brief isSaveEnabled Method to determine if writing of logs to a file is
     * enabled.
     *
     * @return If the method returns true, then writing of logs to files is
     * enabled. Otherwise writing of logs is currently disabled.
     */
    bool isSaveEnabled() const { return saveLogs; }

public slots:
    /**
     * @brief Sets the file name of the log file. Likely to be used AFTER
     * the logChanged signal is emitted, but that is to be determined.
     *
     * @param fileName The new name of the log file.
     *
     * @return This method returns true if the log file name was set successfully.
     */
    bool setLogFileName(const QString &fileName);


    /**
     * @brief setSaveStatus Enable/disable writing logs to a log file.
     *
     * This method can be used to enable/disable writing logs to a given log file.
     * The log file must have been set via a suitable call to the setLogFileName
     * method.
     *
     * \note This method emits the saveStatusChanged() signal.
     *
     * @param saveToFile If this parameter is true, then this method starts writing
     * future logs to the log file (if a log file has been set). If the flag is
     * false, then writing of logs to file is suspended.
     */
    void setSaveStatus(bool saveToFile);

    /**
     * @brief toggleSaveStatus Convenience method to toggle current save status.
     *
     * This is a convenience slot that can be used by GUI/view components to
     * toggle the current log save status.  Invoking this method is the same
     * as calling setSaveStatus(!isSaveEnabled()).
     *
     * \note Calling this method causes the saveStatusChanged signal to be emitted.
     *
     */
    void toggleSaveStatus();

signals:
    void logChanged();
    void logFileNameUpdated();
    void errorSavingLog(const QString& errMsg);
    void saveStatusChanged(bool saveEnabled);

protected:
    virtual void appendLogEntry(const Logger::LogLevel level,
                        const QMessageLogContext &context,
                        const QString &msg) = 0;

protected:
    Log();
    virtual ~Log();

    /**
     * @brief isLogFileGood Convenience method to detect if the log file is good,
     * is ready for writing, and writing has been enabled.
     *
     * \note This method accounts for the status of the saveLogs flag.
     *
     * @return This method returns true if the log stream is ready for writing
     * logs to. Otherwise this method returns false.
     */
    bool isLogStreamGood() const {
        return saveLogs && (logStream.status() == QTextStream::Ok) &&
                (logStream.device() != NULL);
    }

    /**
     * @brief checkLogStream Convenience method to check if the logStream is in
     * good condition and if not, emit an error.
     *
     * This method provides a simple mechanism to check if the logStrema is in a
     * good condition after writing data to it. If the logStream is not in good
     * condition, then this method emits an error message.
     *
     * \note It is meanigful to call this method only after an write operation.
     * Furthermore, the stream should have been checked to be in good condition
     * prior to the write operation as shown in the code fragment below:
     *
     * \code
     *
     * if (isLogStreamGood()) {
     *     logStream << logData.logEntries[0];
     *     // Ensure the data was successfuly written and report any errors.
     *     checkLogStream();
     * }
     *
     * \endcode
     *
     * @return This method returns true if the stream is still in good status.
     * Upon errors this method returns false.
     */
    void checkLogStream();

    /**
     * @brief logFile The actual file to which log entries are to be streamed.
     * This object is not initially used. A valid value is set via the
     * setLogFileName slot/method in this class. This value is used to suitably
     * seed the logStream.
     */
    QFile logFile;

    /**
     * @brief logStream A wrapper around the logFile to streamline saving log
     * entries to the log file. All operations to this stream much check to
     * ensure its status is QTextStream::Ok prior to operating on it. In addition,
     * the Log::checkLogStream() method must be invoked to validate the log stream
     * after an I/O operation.
     */
    QTextStream logStream;

    /**
     * @brief saveLogs Flag to indicate if the logs are to be saved to the file.
     * This flag is used to temporarily enable/disable saving logs to file to
     * ensure that the log files don't exceed their size. This flag is considered
     * by the isLogStreamGood() method.
     */
    bool saveLogs;
};

#endif // LOG_H
