#ifndef LOCAL_SERVER_SESSION_CPP
#define LOCAL_SERVER_SESSION_CPP

#include "LocalServerSession.h"
#include "LocalServerWorkspace.h"
#include "Core.h"

#include <QDir>
#include <QProcess>
#include <QSysInfo>

#include <thread>
#include <chrono>

LocalServerSession::LocalServerSession(Server server, QWidget *parent) :
    ServerSession(server, parent)
{
}

void
LocalServerSession::getOSType() {
    QString os{ Server::UnknownOS };
    QString info{ QSysInfo::kernelType() };

    if (info.contains(Server::Linux, Qt::CaseInsensitive)) {
        os = Server::Linux;
    } else if (info.contains(Server::Unix, Qt::CaseInsensitive)) {
        os = Server::Unix;
    } else if (info.contains(Server::Windows, Qt::CaseInsensitive)) {
        os = Server::Windows;
    } else if (info.contains(Server::OSX, Qt::CaseInsensitive)) {
        os = Server::OSX;
    }

    emit announceOSType(os);
}

void
LocalServerSession::connectToServer() {
    std::cout << "connecting to localhost..." << std::endl;
    using namespace std::literals::chrono_literals;
    std::this_thread::sleep_for(5s);
    std::cout << "connected to localhost." << std::endl;

    test = 0;

    emit connectedToServer(true);
}


//void
//LocalServerSession::disconnectFromServer() {

//}

//int
//LocalServerSession::exec(const QString &command, QString &stdoutput,  QString &stderrmsgs) {
//    QProcess process;
//    process.start(command);
//    process.waitForFinished();
//    stdoutput = process.readAllStandardOutput();
//    stderrmsgs = process.readAllStandardError();

//    return process.exitCode();
//}

//int
//LocalServerSession::exec(const QString &command, QTextEdit& output) {
//    QProcess process;
//    process.start(command);
//    process.waitForFinished();
//    output.append(process.readAllStandardOutput());
//    output.append(process.readAllStandardError());

//    return process.exitCode();
//}

//bool
//LocalServerSession::copy(const QString& srcData, const QString &destDirectory,
//                         const QString &destFileName, const int &mode) {
//    Q_UNUSED(mode);
//    QString newFilePath = destDirectory +
//            (destDirectory.endsWith("/") ? destFileName : "/" + destFileName);
//    return QFile::copy(srcData, newFilePath);
//}

//bool
//LocalServerSession::copy(const QString& destData, const QString &srcDirectory,
//                         const QString& srcFileName) {
//    QString curFilePath = srcDirectory +
//            (srcDirectory.endsWith("/") ? srcFileName : "/" + srcFileName);
//   return QFile::copy(curFilePath, destData);
//}

void
LocalServerSession::mkdir() {
    emit directoryCreated(QDir{ directory }.mkdir(directory));
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

    if (LocalServerWorkspace{ directory }.save() != "") {
        emit serverDataCreated(false);
        return;
    }

    emit serverDataCreated(true);
}

void
LocalServerSession::validate() {
    QDir projectsDir(directory + QDir::separator() + projectsDirName);
    QDir jobsDir(directory + QDir::separator() + jobsDirName);
    QDir scriptsDir(directory + QDir::separator() + scriptsDirName);

    if (!projectsDir.exists() || !projectsDir.isReadable()) {
        emit directoryValidated(false);
        return;
    }

    if (!jobsDir.exists() || !jobsDir.isReadable()) {
        emit directoryValidated(false);
        return;
    }

    if (!scriptsDir.exists() || !scriptsDir.isReadable()) {
        emit directoryValidated(false);
        return;
    }

    if (LocalServerWorkspace{ directory }.load() != "") {
        emit directoryValidated(false);
        return;
    }

    emit directoryValidated(true);
}

//void
//LocalServerSession::setPurpose(const QString &text) {
//    purpose = text;
//}

//void
//LocalServerSession::setPerms(QFile &file, const char &permDigit, const bool owner) {
//    QString perm = &permDigit;
//    uint permNum = perm.toUInt(0, 16);
//    file.setPermissions(file.symLinkTarget(), QFile::Permissions((permNum & 0x4) |
//                                                                 (permNum & 0x2) |
//                                                                 (permNum & 0x1)));

//    Q_UNUSED(owner);
//}

#endif
