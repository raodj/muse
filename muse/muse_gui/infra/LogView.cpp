#include "LogView.h"


LogView::LogView(QWidget *parent) :
    QWidget(parent){
   // this->log = log;
    createToolBar();
    this->shouldSave = false;
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


    connect(changeLogFileName, SIGNAL(triggered()),
            this, SLOT(selectNewLogFile()));
    connect(saveToggleButton, SIGNAL(triggered()), this, SLOT(updateSavePreference()));
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
    saveToggleButton->setIcon(QIcon(":/images/16x16/DontSaveLog.png"));
    saveToggleButton->setEnabled(false);

}

void LogView::selectNewLogFile(){
    QString fileName = QFileDialog::getSaveFileName(this, "Save File", QDir::fromNativeSeparators("/"));

    if(!fileName.isEmpty()){
        emit logFileNameChanged(fileName);
        saveToggleButton->setEnabled(true);
    }
}

void LogView::updateSavePreference(){
    shouldSave = !shouldSave;
    if(shouldSave){
        saveToggleButton->setIcon(QIcon(":/images/16x16/SaveLog.png"));
        emit saveFileNow();
    }
    else saveToggleButton->setIcon(QIcon(":/images/16x16/DontSaveLog.png"));
}
