#ifndef SERVER_CONNECTION_TESTER_H
#define SERVER_CONNECTION_TESTER_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
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


    // For controlling the threads
    static QWaitCondition userHasResponded, passUserData;
    static QMutex mutex, userDataMutex;

signals:


private slots:
    /**
     * @brief interceptRequestForCredentials Intercepts the the SshSocket's
     * request for credentials to prevent the LoginCredentials dialog from
     * appearing on the screen.
     * @param username The pointer to the SshSocket's username.
     * @param passWord The pointer to the SshSocket's password.
     */
    void interceptRequestForCredentials(QString *username, QString *passWord);

    /**
     * @brief promptUser Displays user prompts from the threaded SSH connection
     * to allow the SSH class to perform its connection correctly.
     * @param windowTitle The window title of the QMessageBox.
     * @param text The primary text of the QMessageBox.
     * @param informativeText The informative text for the QMessageBox.
     * @param detailedText The detailed text for the QMessageBox.
     * @param userChoice The button pressed to close the QMessageBox.
     */
    void promptUser(const QString &windowTitle, const QString& text,
                    const QString &informativeText,
                    const QString &detailedText, int* userChoice);


private:
    bool success;
    SshSocket* connection;
    QString userName, password, hostName;
    int portNumber;

};

#endif // SERVERCONNECTIONTESTER_H
