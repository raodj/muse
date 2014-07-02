#ifndef MUSETHREAD_H
#define MUSETHREAD_H

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
