#include "Core.h"
#include "LocalServerSession.h"
#include <QDir>
#include <QProcess>

LocalServerSession::LocalServerSession(Server* server, QWidget *parent, QString purpose) :
    ServerSession(server, parent), purpose(purpose) {
}


void
LocalServerSession::connectToServer() {

}


void
LocalServerSession::disconnectFromServer() {

}

int
LocalServerSession::exec(const QString &command, QString &stdoutput,  QString &stderrmsgs) {
    QProcess process; //= new QProcess();
    process.start(command);
    process.waitForFinished();
    stdoutput = process.readAllStandardOutput();
    stderrmsgs = process.readAllStandardError();

    return process.exitCode();

}

int
LocalServerSession::exec(const QString &command, QTextEdit& output) {

    QProcess process; //= new QProcess();
    process.start(command);
    process.waitForFinished();
    output.append(process.readAllStandardOutput());
    output.append(process.readAllStandardError());

    return process.exitCode();
}

bool
LocalServerSession::copy(const QString& srcData, const QString &destDirectory,
                              const QString &destFileName, const int &mode) {
    QString newFilePath = destDirectory +
            (destDirectory.endsWith("/") ? destFileName : "/" + destFileName);
    return QFile::copy(srcData, newFilePath);
    Q_UNUSED(mode);
}

bool
LocalServerSession::copy(const QString& destData, const QString &srcDirectory,
                              const QString& srcFileName) {

    QString curFilePath = srcDirectory +
            (srcDirectory.endsWith("/") ? srcFileName : "/" + srcFileName);
   return QFile::copy(curFilePath, destData);
}

void
LocalServerSession::mkdir(const QString &directory) {
    QDir dir(directory);
    emit directoryCreated(dir.mkdir(directory));
}

//void
//LocalServerSession::rmdir(const QString &directory) {
//    QDir dir(directory);
 //   emit directoryRemoved(dir.rmdir(directory));
//}

void
LocalServerSession::dirExists(const QString &directory) {
    emit directoryExists(QDir(directory).exists());
}

void
LocalServerSession::createServerData(const QString& directory) {
    /// TODO: A class to store information from the Server still needs
    /// to be made.  The Server will store its info as xml, so this class
    /// will work in a way similar to Workspace.  For now we will just create
    /// the necessary directories

    QDir projectsDir(directory + server->separator() + "projects");
    QDir jobsDir(directory + server->separator() + "jobs");
    QDir scriptsDir(directory + server->separator() + "scripts");

    if (!projectsDir.exists() && !projectsDir.mkdir(projectsDir.absolutePath())) {
        emit serverDataCreated(false);
        return;
    }

    if (!jobsDir.exists() && !jobsDir.mkdir(jobsDir.absolutePath())) {
        emit serverDataCreated(false);
        return;
    }

    if (!scriptsDir.exists() && !scriptsDir.mkdir(scriptsDir.absolutePath())) {
        emit serverDataCreated(false);
        return;
    }

    emit serverDataCreated(true);
}

void
LocalServerSession::validate(const QString &directory) {
    /// TODO: A class to store information from the Server still needs
    /// to be made.  The Server will store its info as xml, so this class
    /// will work in a way similar to Workspace.  For now we will just create
    /// the necessary directories

    QDir projectsDir(directory + server->separator() + "projects");
    QDir jobsDir(directory + server->separator() + "jobs");
    QDir scriptsDir(directory + server->separator() + "scripts");

    if (!projectsDir.exists()) {
        emit directoryValidated(false);
        return;
    }

    if (!jobsDir.exists()) {
        emit directoryValidated(false);
        return;
    }

    if (!scriptsDir.exists()) {
        emit directoryValidated(false);
        return;
    }

    emit directoryValidated(true);
}

void
LocalServerSession::setPurpose(const QString &text) {
    purpose = text;
}

void
LocalServerSession::setPerms(QFile &file, const char &permDigit, const bool owner) {
    QString perm = &permDigit;
    uint permNum = perm.toUInt(0, 16);
    file.setPermissions(file.symLinkTarget(), QFile::Permissions((permNum & 0x4) |
                                                                 (permNum & 0x2) |
                                                                 (permNum & 0x1)));

    Q_UNUSED(owner);
}
