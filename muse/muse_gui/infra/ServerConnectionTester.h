#ifndef SERVER_CONNECTION_TESTER_H
#define SERVER_CONNECTION_TESTER_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include "SshSocket.h"
#include "ThreadedConnectionGUI.h"

/**
 * @brief The ServerConnectionTester class is used to verify that the
 * server the user has chosen to add to the workspace exists and the
 * information provided is correct. This class runs on a separate
 * thread from the main application.
 */
class ServerConnectionTester : public QThread {
    Q_OBJECT
public:
    /**
     * @brief ServerConnectionTester The primary constructor for the
     * ServerConnectionTester. The parameters are used to preserve information
     * about the desired server to connect to for use by this Tester.
     * @param userName The username credential for the server
     * @param password The password credential for the server
     * @param hostName The name of the server
     * @param portNumber The port number for the connection to the server.
     * @param parent The parent QObject this ServerConnectionTester belongs to.
     */
    ServerConnectionTester(QString userName,
                           QString password, QString hostName,
                           const int portNumber = 22, QObject *parent = 0);
    /**
     * @brief run The overriden run method that is required for threaded classes.
     * This class tries to connect to the server with the given credentials that
     * are stored as instance variables of this ServerConnectionTester.
     */
    void run();

    /**
     * @brief getResult Returns the result of the attempt to connect with the
     * server.
     * @return The result of the test.
     */
    bool getResult();




signals:

    /**
     * @brief exceptionThrown Alerts the main Qt gui thread that an
     * exception was thrown and that the program needs to display a
     * warning to the user explaining what happened.
     * @param e The exception thrown.
     */
    void exceptionThrown(const QString& message, const QString& genErrorMessage,
                         const QString& exceptionDetails);

private slots:
    /**
     * @brief interceptRequestForCredentials Intercepts the the SshSocket's
     * request for credentials to prevent the LoginCredentials dialog from
     * appearing on the screen.<br/>
     *
     * Normally, this would be handled by the ThreadedConnectionGUI,
     * but since we don't want to create a Server variable yet (if this fails),
     * we will handle the credentials request here, since we have access
     * to the password.
     *
     * @param username The pointer to the SshSocket's username.
     * @param passWord The pointer to the SshSocket's password.
     */
    void interceptRequestForCredentials(QString* username, QString* passWord);

private:
    bool success;
    SshSocket* connection;
    QString userName, password, hostName;
    int portNumber;
    ThreadedConnectionGUI tcg;

};

#endif // SERVERCONNECTIONTESTER_H
