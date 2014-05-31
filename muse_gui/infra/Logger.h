#ifndef LOGGER_H
#define LOGGER_H

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

#include <QDebug>

/**
 * @brief The Logger class that is used to temporarily buffer information
 * and then generate a programmer or user log entry.
 *
 * This class is meant to be used along with a few macros (defined in this
 * header file) to conveniently generate programmer and user logs throughout
 * MUSE GUI code.  This class essentially serves as a temporary buffer to hold
 * information associated with a log entry and finally cut the log entry.
 *
 * Typically, the Logger class will not be directly referenced in MUSE source
 * code. Instead the convenience macros defined in this class must be used to
 * generated log entries. An short example of using this class for generating
 * programmer and user log entries is shown below:
 *
 * \code
 *
 * // Assume this fragment is in a .cpp file
 *
 * // Include header for sub-system
 * #include "Infrastructure.h"
 *
 * // several methods may be defined here.
 *
 * SomeClass::cleanInput(const QString& str) {
 *    // ... some code goes here ...
 *
 *    // Cut a programmer log...
 *    progLog() << "Unhandled string encountered: " << str;
 *
 *    // Cut a user log at default NOTICE log level
 *    userLog() << "An invalid string was encountered in input!";
 *
 *    // Cut a user log at WARNING log level
 *    userLog(Logger::WARNING) << "The input may not be correctly handled!";
 * }
 *
 * // some more methods may be defined here.
 *
 * \endcode
 *
 */
class Logger : public QObject {
    Q_OBJECT
public:
    /** Enumeration of various log levels that are supported.
     *
     * The following enumeration provides a mechanism to cut more
     * meanigful logs, specifically in user logs.  Essentially these
     * log levels map
     * to suitable icons in the GUI display.  The values are
     * interpreted in the following manner:
     *
     * <UL>
     *
     *  <LI> \b VERBOSE: This log level must be used for generating more
     * detailed information, such as those that may be generated during
     * routine processing to provide the user with some additional
     * ongoing information about progress of tasks. Typically a user will
     * not pay much attention to this log level and these may be filtered
     * out based on the log-level set by the user.</LI>
     *
     * <LI> \b NOTICE: This log level that must be used for displaying
     * general information to the user.  This log level is used for
     * confirming starting, progress, and successful-completion (for
     * unsuccessful completion use other appropriate log levels) of
     * routine operations that occur in the system. These are events
     * that the user may occasionally some attention to, particularly
     * for troubleshooting purposes. </LI>
     *
     * <LI> \b WARNING: Display some warning information indicating
     * potential but recoverable problems during system operations. The
     * user is expected to pay special attention to these messages.
     * Consequently, it is imperative to include sufficient information
     * to facililate the user to troubleshoot issues.</LI>
     *
     * <LI> \b ERROR: Display fatal error that may have occured
     * during operations and the operation has been abandoned. The
     * user is expected to pay special attention to these messages.
     * Consequently, it is imperative to include sufficient information
     * to facililate the user to troubleshoot issues.</LI>
     *
     * </UL>
    */
    enum LogLevel{LOG_VERBOSE, LOG_NOTICE, LOG_WARNING, LOG_ERROR};

    /**
     * @brief toString Method to obtain string representation of a LogLevel.
     *
     * This is a convenience method that can be used to obtain the
     * string representation of various log level values enumerated in
     * the LogLevel enumeration. For instance given Logger::VERBOSE
     * this method returns the string "Verbose". If the log level specified
     * is invalid, this method always returns "Error" as the log level.
     *
     * @param level The log level to be converted to a string.
     *
     * @return The string representation of the given log level.
     */
    static QString toString(const LogLevel level);


    /**
     * @brief Logger Constructor used to create a logger object that cuts
     * a programmer or user log entry in its destructor.
     *
     * The constructor provides a comprehensive set of parameters to
     * initialize the various instance variables in this class. The information
     * is used to generate a programmer or user log entry in the destructor
     * by calls to ProgrammerLog::appendLogEntry or UserLog::appendLogEntry.
     *
     * @note Typically the constructor would never be directly used in the
     * source code.  Instead, convenience macros progLog() or userLog() defined
     * in this header file must be used to create log entries.
     *
     * @param level The log level at which the log entry is to be created.
     * This value is applicable only to user logs. For programmer logs this
     * value is ignored and the logs are always generated.
     *
     * @param userLogEntry A boolean flag to indicate if the log entry is
     * a programmer or a user log.
     *
     * @param file The source file name (typically automatically generated
     * when the progLog() or userLog() macros are used) from where the log
     * entry is being created. This information is alays used  in the
     * programmer log but selectively used in the user log.
     *
     * @param line The source file line number (typically automatically
     * generated when the progLog() or userLog() macros are used) from
     * where the log entry is being created. This information is alays
     * used in the programmer log but selectively used in the user log.
     *
     * @param function The method or function from in which this log
     * entry is being generated. Typically this value is automatically
     * generated using suitable compiler variables when the progLog()
     * or userLog() macros are used) from. This information is always
     * used in the programmer log but selectively used in the user log.
     *
     * @param component A logical subsystem/component name from where
     * the log entry is being generated. This information is currently
     * used only in the user log.
     */
    inline Logger(const LogLevel level = LOG_NOTICE, bool userLogEntry = false,
                  const char* file = "", int line = 0, const char* function = "",
                  const char* component = "") :
        context(file, line, function, component), logLevel(level),
        isUserLogEntry(userLogEntry) {}

    /** The destructor for this class.
     *
     * The destructor uses the information accumulated in the instance
     * variables in this class to finally cut a programmer or user log.
     * The destructor essentially calls ProgrammerLog::appendLogEntry or
     * UserLog::appendLogEntry to cut the appropriate logs.
     */
    ~Logger();

    /**
     * @brief log Convenience method typically used to generate programmer
     * logs or default user logs.
     *
     * This method is the primary interface method for generating log entries
     * using a Logger object. This method provides the following two features:
     *
     *   -# It provides an optional format string (similar to format string
     *   used with the printf function) along with parameters to conveniently
     *   generate a log entry.
     *
     *   -# It returns a QDebug stream (backed by an internal QString that
     *   contains the actual log entry) that already contains the formatted
     *   information (if any) from the previous feature. Additional log
     *   information can be conveniently appended/inserted into this stream.
     *
     * @note Typically the this method would never be directly used in the
     * source code.  Instead, convenience macros progLog() or userLog() defined
     * in this header file must be used to create log entries.
     *
     * @param formatStr An optional (if NULL, this string and any additional
     * parameters are ignored) format string to generate initial log entry.
     * The format string is identical to the format string used with printf.
     * The remainder of the parameters must match any format specifiers
     * (such as: \c %d) specified in the format string.
     *
     * @return This method always returns a valid QDebug stream that
     * already contains the formatted information from formatStr (if any).
     * Additional log information can be appended to the log entry by
     * inserting information into the QDebug stream returned by this method.
     */
    QDebug log(const char *formatStr = NULL, ...) const;

    /**
     * @brief log Convenience method typically used to generate user
     * logs at various log levels.
     *
     * This method is the primary interface method for generating log entries
     * using a Logger object. This method provides similar features as the
     * other log() method in this class except it adds an additional
     * parameter to customize the log levels at which the log entry is to
     * be generated. The log level is applicable to just user logs (for
     * programmer logs this value is ignored).
     *
     * @note Typically the this method would never be directly used in the
     * source code.  Instead, convenience macros progLog() or userLog() defined
     * in this header file must be used to create log entries.
     *
     * @param level The log level at which the log entry is to be generated.
     * This value must be a valid enumerated value defined in
     * Logger::LogLevel.
     *
     * @param formatStr An optional (if NULL, this string and any additional
     * parameters are ignored) format string to generate initial log entry.
     * The format string is identical to the format string used with printf.
     * The remainder of the parameters must match any format specifiers
     * (such as: \c %d) specified in the format string.
     *
     * @return This method always returns a valid QDebug stream that
     * already contains the formatted information from formatStr (if any).
     * Additional log information can be appended to the log entry by
     * inserting information into the QDebug stream returned by this method.
     */
    QDebug log(const LogLevel level, const char *formatStr = NULL, ...) const;

private:
    /**
     * @brief context The log context that contains source code information.
     *
     * This instance variable is used to maintain the source code information
     * from where the logs are being generated, namely:
     *
     *   - The source file name (example: \c Logger.cpp)
     *   - The line number in the source file
     *   - The full method name from where the log entry is being created
     *   - The name of the muse component, set by calls to the
     *     BEGIN_MUSE_SUBSYSTEM and END_MUSE_SUBSYSTEM methods.
     *
     * This value is set when a Logger instance is created and is never
     * changed during the life time of this object.
     */
    const QMessageLogContext context;

    /**
     * @brief logMsg The string that contains the log message to be cut.
     *
     * This string is initialized with values from any format strings
     * specified. In addition, this string is used as the backing store
     * for the QDebug stream returned by the various log() methods to
     * buffer additional logging information.
     */
    mutable QString logMsg;

    /**
     * @brief logLevel The log level associated with this log entry.
     *
     * This value must be one of the valid enumerated log level constants.
     */
    mutable LogLevel logLevel;

    /**
     * @brief isUserLogEntry Flag to indicate if this is a programmer log
     * or a user log entry.
     *
     * If this flag is \c true, then the log entry is assumed to be a user log
     * entry (and the log entry is finally generated using
     * UserLog::appendLogEntry). If the flag is \c false, the log entry is
     * assumed to be a programmer log entry.
     */
    const bool isUserLogEntry;
};

/** \def progLog
  *
  * A convenience macro to generate a programmer log entry. This macro is
  * designed to automaticlaly fill-in several default values and set-up
  * the information necessary for the context. This macro creates a local
  * Logger object and invokes the log() method on the Logger. Any parameters
  * to this macro are automatically passed as parameters to the log() method.
  * This macro must be used as shown in the following example:
  *
  * \code
  *
  * // Assume this fragment is in a .cpp file
  *
  * // Include header for sub-system
  * #include "Infrastructure.h"
  *
  * // several methods may be defined here.
  *
  * SomeClass::cleanInput(const QString& str) {
  *    // ... some code goes here ...
  *
  *    // Cut a programmer log...
  *    progLog() << "Unhandled string encountered: " << str;
  *
  * }
  *
  * // some more methods may be defined here.
  *
  * \endcode
  */
#define progLog Logger(Logger::LOG_NOTICE, false, __FILE__, __LINE__, \
    Q_FUNC_INFO, MUSE_SUBSYS).log

/** \def userLog
  *
  * A convenience macro to generate a user log entry. This macro is
  * designed to automaticlaly fill-in several default values and set-up
  * the information necessary for the context. This macro creates a local
  * Logger object and invokes the log() method on the Logger. Any parameters
  * to this macro are automatically passed as parameters to the log() method.
  * This macro must be used as shown in the following example:
  *
  * \code
  *
  * // Assume this fragment is in a .cpp file
  *
  * // Include header for sub-system
  * #include "Infrastructure.h"
  *
  * // several methods may be defined here.
  *
  * SomeClass::cleanInput(const QString& str) {
  *    // ... some code goes here ...
  *
  *    // Cut a user log at default NOTICE log level
  *    userLog() << "An invalid string was encountered in input!";
  *
  *    // Cut a user log at WARNING log level
  *    userLog(Logger::WARNING) << "The input may not be correctly handled!";
  *
  * }
  *
  * // some more methods may be defined here.
  *
  * \endcode
  */
#define userLog Logger(Logger::LOG_NOTICE, true, __FILE__, __LINE__, \
    Q_FUNC_INFO, MUSE_SUBSYS).log

#endif // LOGGER_H
