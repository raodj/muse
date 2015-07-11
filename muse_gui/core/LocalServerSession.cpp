#ifndef LOCAL_SERVER_SESSION_CPP
#define LOCAL_SERVER_SESSION_CPP

#include "LocalServerSession.h"
#include "LocalServerWorkspace.h"
#include "Core.h"

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
    QProcess process;
    process.start(command);
    process.waitForFinished();
    stdoutput = process.readAllStandardOutput();
    stderrmsgs = process.readAllStandardError();

    return process.exitCode();
}

int
LocalServerSession::exec(const QString &command, QTextEdit& output) {
    QProcess process;
    process.start(command);
    process.waitForFinished();
    output.append(process.readAllStandardOutput());
    output.append(process.readAllStandardError());

    return process.exitCode();
}

bool
LocalServerSession::copy(const QString& srcData, const QString &destDirectory,
                         const QString &destFileName, const int &mode) {
    Q_UNUSED(mode);
    QString newFilePath = destDirectory +
            (destDirectory.endsWith("/") ? destFileName : "/" + destFileName);
    return QFile::copy(srcData, newFilePath);
}

bool
LocalServerSession::copy(const QString& destData, const QString &srcDirectory,
                         const QString& srcFileName) {
    QString curFilePath = srcDirectory +
            (srcDirectory.endsWith("/") ? srcFileName : "/" + srcFileName);
   return QFile::copy(curFilePath, destData);
}

void
LocalServerSession::mkdir() {
    QDir dir{ directory };
    emit directoryCreated(dir.mkdir(directory));
}

void
LocalServerSession::dirExists() {
    emit directoryExists(QDir{ directory }.exists());
}

void
LocalServerSession::createServerData() {
    QDir projectsDir(directory + QDir::separator() + projectsDirName);
    QDir jobsDir(directory + QDir::separator() + jobsDirName);
    QDir scriptsDir(directory + QDir::separator() + scriptsDirName);

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

    if (LocalServerWorkspace{ directory }.save() == "") {
        emit serverDataCreated(true);
    } else {
        emit serverDataCreated(false);
    }
}

void
LocalServerSession::validate() {
    QDir projectsDir(directory + QDir::separator() + projectsDirName);
    QDir jobsDir(directory + QDir::separator() + jobsDirName);
    QDir scriptsDir(directory + QDir::separator() + scriptsDirName);

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

    if (LocalServerWorkspace{ directory }.load() == "") {
        emit directoryValidated(true);
    } else {
        emit directoryValidated(false);
    }
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

#endif
