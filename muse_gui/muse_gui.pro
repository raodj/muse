#-------------------------------------------------
#
# Project created by QtCreator 2013-05-24T13:28:22
#
#-------------------------------------------------

QT       += core gui xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -L"$$_PRO_FILE_PWD_/libs/" -lssh2


TARGET = muse_gui
TEMPLATE = app

INCLUDEPATH += infra
INCLUDEPATH += infra/xml
INCLUDEPATH += infra/ssh
INCLUDEPATH += workspace
INCLUDEPATH += core
INCLUDEPATH += core/filediag
INCLUDEPATH += core/firstRunWizard
INCLUDEPATH += /usr/local/include

# DEFINES += QT_NO_DEBUG_STREAM

SOURCES +=\
    infra/UserLogView.cpp \
    infra/UserLogTableModel.cpp \
    infra/UserLog.cpp \
    infra/ProgrammerLogView.cpp \
    infra/ProgrammerLog.cpp \
    infra/Logger.cpp \
    infra/Log.cpp \
    infra/DnDTabWidget.cpp \
    infra/DnDTabMimeData.cpp \
    infra/DnDTabBar.cpp \
    core/MainWindow.cpp \
    core/main.cpp \
    core/RemoteServerSession.cpp \
    core/LocalServerSession.cpp \
    core/ServerSession.cpp \
    infra/LogView.cpp \
    infra/xml/XMLParser.cpp \
    infra/xml/XMLElementInfo.cpp \
    infra/xml/XMLElement.cpp \
    workspace/Workspace.cpp \
    workspace/ServerList.cpp \
    workspace/Server.cpp \
    infra/xml/XMLRootElement.cpp \
    infra/ssh/SshSocket.cpp \
    infra/ssh/SSHKnownHosts.cpp \
    infra/ssh/SshException.cpp \
    infra/ssh/SFtpDir.cpp \
    infra/ssh/SFtpChannel.cpp \
    infra/ssh/LoginCredentialsDialog.cpp \
    core/filediag/RemoteFSHelper.cpp \
    core/filediag/LocalFSHelper.cpp \
    core/filediag/LineEditDelegate.cpp \
    core/filediag/FSMAsyncHelper.cpp \
    core/filediag/FSHelperCommon.cpp \
    core/filediag/FSEntry.cpp \
    core/filediag/FileSystemModel.cpp \
    core/filediag/CustomFileDialog.cpp \
    core/filediag/DirFilterProxyModel.cpp \
    core/MUSEWizard.cpp \
    core/firstRunWizard/FirstRunWizard.cpp \
    core/firstRunWizard/WelcomePage.cpp \
    core/firstRunWizard/LicensePage.cpp \
    core/firstRunWizard/WorkSpacePage.cpp \
    core/MUSEApplicationDirectory.cpp


HEADERS  += \
    infra/UserLogView.h \
    infra/UserLogTableModel.h \
    infra/UserLog.h \
    infra/ProgrammerLogView.h \
    infra/ProgrammerLog.h \
    infra/Logger.h \
    infra/Log.h \
    infra/Infrastructure.h \
    infra/DnDTabWidget.h \
    infra/DnDTabMimeData.h \
    infra/DnDTabBar.h \
    core/Version.h \
    core/MainWindow.h \
    core/Core.h \
    core/RemoteServerSession.h \
    core/LocalServerSession.h \
    core/ServerSession.h \
    infra/LogView.h \
    infra/xml/XMLRootElement.h \
    infra/xml/XMLParser.h \
    infra/xml/XMLElementInfo.h \
    infra/xml/XMLElement.h \
    workspace/Workspace.h \
    workspace/ServerList.h \
    workspace/Server.h \
    infra/ssh/SshSocket.h \
    infra/ssh/SSHKnownHosts.h \
    infra/ssh/SshException.h \
    infra/ssh/SFtpDir.h \
    infra/ssh/SFtpChannel.h \
    infra/ssh/LoginCredentialsDialog.h \
    core/filediag/RemoteFSHelper.h \
    core/filediag/LocalFSHelper.h \
    core/filediag/LineEditDelegate.h \
    core/filediag/FSMAsyncHelper.h \
    core/filediag/FSHelperCommon.h \
    core/filediag/FSHelper.h \
    core/filediag/FSEntry.h \
    core/filediag/FileSystemModel.h.bak \
    core/filediag/FileSystemModel.h \
    core/filediag/FileSystemModel.cpp.bak \
    core/filediag/CustomFileDialog.h \
    core/filediag/DirFilterProxyModel.h \
    core/MUSEWizard.h \
    core/firstRunWizard/FirstRunWizard.h \
    core/firstRunWizard/WelcomePage.h \
    core/firstRunWizard/LicensePage.h \
    core/firstRunWizard/WorkSpacePage.h \
    core/MUSEApplicationDirectory.h


OTHER_FILES += \
    GPL.txt

RESOURCES += \
    muse_gui.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../../usr/local/lib/release/ -lssh2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../../usr/local/lib/debug/ -lssh2
else:unix: LIBS += -L$$PWD/../../../../../../../../usr/local/lib/ -lssh2

INCLUDEPATH += $$PWD/../../../../../../../../usr/local/lib
DEPENDPATH += $$PWD/../../../../../../../../usr/local/lib
