#include "LogView.h"
#include <QAction.h>
LogView::LogView(QWidget *parent, Log *log) :
    QWidget(parent){
    this->log = log;
    createToolBar();
}

void LogView::createToolBar(){
    logToolBar = new QToolBar();
    initializeLabel();
    initializeFileNameDisplay();
    initializeActions();

    logToolBar->addWidget(fileNameLabel);
    logToolBar->addWidget(fileNameDisplay);
    logToolBar->addAction(changeLogFileName);
    logToolBar->addAction(saveToggleButton);
}

void LogView::updateFileName(){
    fileNameDisplay->setText(log->getLogFileName());
}

void LogView::initializeLabel(){
    fileNameLabel = new QLabel("Log filename: ");
}

void LogView::initializeFileNameDisplay(){
    fileNameDisplay = new QLineEdit("<none set>");
    fileNameDisplay->setReadOnly(true);
}

void LogView::initializeActions(){
    changeLogFileName = new QAction("Change log filename", this);
    changeLogFileName->setIcon(QIcon(":/images/16x16/ChangeLogFile.png"));

    saveToggleButton = new QAction("Toggle log saving", this);
    saveToggleButton->setIcon(QIcon(":/images/16x16/SaveLog.png"));
    saveToggleButton->setEnabled(false);
}
