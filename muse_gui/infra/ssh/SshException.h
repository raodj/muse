#ifndef SSH_EXCEPTION_H
#define SSH_EXCEPTION_H

#include <stdexcept>
#include <QString>
#include <QAbstractSocket>

// Forward declaration to avoid circular dependency
class SshSocket;

#define SSH_EXP2(ssh, msg) \
    SshException(ssh, msg, -1, QAbstractSocket::UnknownSocketError,\
    __FILE__, __LINE__, __FUNCTION__)

#define SSH_EXP(ssh, msg, sshErrCode, netErrCode) \
    SshException(ssh, msg, sshErrCode, netErrCode,\
    __FILE__, __LINE__, __FUNCTION__)

class SshException : public std::exception {
public:
    SshException(SshSocket& ssh, const QString& msg, int sshErrorCode,
                 QAbstractSocket::SocketError networkErrorCode,
                 const QString& fileName, const int lineNumber,
                 const QString& methodName) throw ();
    ~SshException() throw () {}
    //SshException(const SshException& e);

    SshSocket& getSocket() { return ssh; }
    const QString& getMessage() const throw () { return msg; }
    int getSshErrorCode() const throw () { return sshErrorCode; }
    QAbstractSocket::SocketError getNetworkErrorCode() const throw ()
    { return networkErrorCode; }
    const char* what() const throw () { return msg.toStdString().c_str(); }
    const QString& getFileName() const { return fileName; }
    int getLineNumber() const { return lineNumber; }
    const QString& getMethodName() const { return methodName; }

    static void show(const SshException &exp, QWidget *parent = NULL);
    QString getErrorDetails() const { return ErrorDetails; }
    QString getGenericErrorMessage() const  { return GenericErrorMessage; }

protected:
    static const QString GenericErrorMessage;
    static const QString ErrorDetails;

private:
    SshSocket& ssh;
    const QString msg;
    const int sshErrorCode;
    const QAbstractSocket::SocketError networkErrorCode;
    const QString fileName;
    const int lineNumber;
    const QString methodName;
};

#endif // SSH_EXCEPTION_H
