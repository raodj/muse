#ifndef SERVER_CONNECTION_TESTER_H
#define SERVER_CONNECTION_TESTER_H

#include <QThread>
#include "SshSocket.h"

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
                           const int portNumber = 22, QWidget *mainThread = 0, QObject *parent = 0);
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

    QWidget* getParentWidget();

signals:
    /**
     * @brief passUserCredentials Passes the server information to the SshSocket
     * instance variable's getCredentials() slot so that the tester
     * can connect to the server.
     * @param ssh The ssh socket trying to make a connection
     * @param hostInfo The known host info
     * @param userID The username given by the user
     * @param password The password given by the user
     * @param cancel
     */
    void passUserCredentials(SshSocket& ssh, const libssh2_knownhost* hostInfo,
                             QString& userID, QString& password, bool& cancel);

private slots:
    /**
     * @brief interceptRequestForCredentials Intercepts the the SshSocket's
     * request for credentials to prevent the LoginCredentials dialog from
     * appearing on the screen. This method emits passUserCredentials()
     * and this signal will be captured by the SshSocket's getCredentials()
     * slot.
     * @param ssh The SshSocket attempting to make a connection.
     * @param hostInfo The known host info.
     * @param userID The user name given by the user
     * @param password The password given by the user
     * @param cancel
     */
    void interceptRequestForCredentials(SshSocket &ssh, const libssh2_knownhost *hostInfo,
                                        QString &userID, QString &password, bool &cancel);

private:
    bool success;
    SshSocket* connection;
    QString userName, password, hostName;
    int portNumber;
    QWidget* ptrToMainThread;
};

#endif // SERVERCONNECTIONTESTER_H
