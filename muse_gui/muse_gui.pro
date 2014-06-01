#-------------------------------------------------
#
# Project created by QtCreator 2013-05-24T13:28:22
#
#-------------------------------------------------

QT       += core gui xmlpatterns

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = muse_gui
TEMPLATE = app

INCLUDEPATH += infra
INCLUDEPATH += infra/xml
INCLUDEPATH += workspace
INCLUDEPATH += core

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
    infra/xml/XMLRootElement.cpp


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
    workspace/Server.h


OTHER_FILES += \
    GPL.txt

RESOURCES += \
    muse_gui.qrc
