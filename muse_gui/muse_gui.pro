#-------------------------------------------------
#
# Project created by QtCreator 2013-05-24T13:28:22
#
#-------------------------------------------------

QT+= core gui xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

LIBS += -L"$$_PRO_FILE_PWD_/libs/" -lssh2

# Setup Linux specific configuration flag
unix:!macx {
    CONFIG += c++1y
    QMAKE_CXXFLAGS += -std=c++1y
}

TARGET = muse_gui
TEMPLATE = app

INCLUDEPATH += infra
INCLUDEPATH += infra/xml
INCLUDEPATH += infra/ssh
INCLUDEPATH += workspace
INCLUDEPATH += core
INCLUDEPATH += core/filediag
INCLUDEPATH += core/firstRunWizard
INCLUDEPATH += core/jobWizard
INCLUDEPATH += core/serverWizard
INCLUDEPATH += core/projectWizard
INCLUDEPATH += core/workspaceWizard
INCLUDEPATH += views

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
    core/firstRunWizard/AppDirPage.cpp \
    core/MUSEGUIApplication.cpp \
    views/View.cpp \
    views/ServerListView.cpp \
    views/ServerListTableModel.cpp \
    core/serverWizard/ServerWizard.cpp \
    core/serverWizard/ServerTypePage.cpp \
    core/serverWizard/ServerInfoPage.cpp \
    core/serverWizard/ServerSummaryPage.cpp \
    core/OverviewPage.cpp \
    infra/ServerConnectionTester.cpp \
    infra/ThreadedConnectionGUI.cpp \
    core/MUSEThread.cpp \
    infra/ssh/SshChannel.cpp \
    core/projectWizard/ProjectWizard.cpp \
    core/projectWizard/ProjectDataPage.cpp \
    core/projectWizard/ServerSelectionPage.cpp \
    core/projectWizard/ProjectSummaryPage.cpp \
    workspace/Project.cpp \
    workspace/ProjectList.cpp \
    core/jobWizard/JobWizard.cpp \
    core/jobWizard/JobInformationPage.cpp \
    core/jobWizard/ServerSetupPage.cpp \
    core/jobWizard/JobSummaryPage.cpp \
    core/filediag/DirectoryNameDialog.cpp \
    core/jobWizard/PBSJobFileCreator.cpp \
    core/jobWizard/SubmitPage.cpp \
    workspace/Job.cpp \
    workspace/JobList.cpp \
    views/GeospatialView.cpp \
    views/GeospatialWidget.cpp \
    core/workspaceWizard/WorkspaceSelectPage.cpp \
    core/workspaceWizard/WorkspaceDialog.cpp


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
    core/filediag/FileSystemModel.h \
    core/filediag/CustomFileDialog.h \
    core/filediag/DirFilterProxyModel.h \
    core/MUSEWizard.h \
    core/firstRunWizard/FirstRunWizard.h \
    core/firstRunWizard/WelcomePage.h \
    core/firstRunWizard/LicensePage.h \
    core/firstRunWizard/AppDirPage.h \
    core/MUSEGUIApplication.h \
    views/View.h \
    views/ServerListView.h \
    views/ServerListTableModel.h \
    core/serverWizard/ServerWizard.h \
    core/serverWizard/ServerTypePage.h \
    core/serverWizard/ServerInfoPage.h \
    core/serverWizard/ServerSummaryPage.h \
    core/OverviewPage.h \
    infra/ServerConnectionTester.h \
    infra/ThreadedConnectionGUI.h \
    core/RSSAsyncHelper.h \
    core/RSSAsyncHelper.ipp \
    core/MUSEThread.h \
    infra/ssh/SshChannel.h \
    core/projectWizard/ProjectWizard.h \
    core/projectWizard/ProjectDataPage.h \
    core/projectWizard/ServerSelectionPage.h \
    core/projectWizard/ProjectSummaryPage.h \
    workspace/Project.h \
    workspace/ProjectList.h \
    core/jobWizard/JobWizard.h \
    core/jobWizard/JobInformationPage.h \
    core/jobWizard/ServerSetupPage.h \
    core/jobWizard/JobSummaryPage.h \
    core/filediag/DirectoryNameDialog.h \
    core/jobWizard/PBSJobFileCreator.h \
    core/jobWizard/SubmitPage.h \
    workspace/Job.h \
    workspace/JobList.h \
    views/GeospatialView.h \
    views/GeospatialWidget.h \
    core/workspaceWizard/WorkspaceSelectPage.h \
    core/workspaceWizard/WorkspaceDialog.h \
    infra/xml/XMLMetaTypeHelper.h

RESOURCES += \
    muse_gui.qrc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../../../../../../../usr/local/lib/release/ -lssh2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../../../../../../../usr/local/lib/debug/ -lssh2
else:unix: LIBS += -L$$PWD/../../../../../../../../usr/local/lib/ -lssh2

INCLUDEPATH += $$PWD/../../../../../../../../usr/local/lib
DEPENDPATH += $$PWD/../../../../../../../../usr/local/lib
