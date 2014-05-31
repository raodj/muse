#include "ProgrammerLogView.h"
#include "ProgrammerLog.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <iostream>

ProgrammerLogView::ProgrammerLogView(QWidget *parent) :
    LogView(parent), logDisplay(this) {

    ProgrammerLog& log = ProgrammerLog::get();

    logDisplay.setReadOnly(true);
    logDisplay.setWordWrapMode(QTextOption::NoWrap);
    // Organize components in this widget for display.
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(logToolBar);
    layout->addWidget(&logDisplay, 100);
    this->setLayout(layout);
    // Handle signals about changes in log
    connect(&log, SIGNAL(logChanged()), this, SLOT(updateLog()));
    // Setup initial data in programmer log
    updateLog();

    connect(this, SIGNAL(logFileNameChanged(const QString &)),
            &log, SLOT(setLogFileName(const QString &)));

    connect(&log, SIGNAL(logFileNameUpdated()),
            this, SLOT(updateFileName()));


    //Connect signal to detect an error in saving the log.
    connect(&log, SIGNAL(errorSavingLog(QString)),
            this, SLOT(saveErrorNotification(QString)));
    //connect(this, SIGNAL(saveFileNow()), this, SLOT(callSave()));
}

void
ProgrammerLogView::updateLog() {
    logDisplay.setText(ProgrammerLog::get().getEntries());
    // Scroll log display to bottom (does not work in X11)
    QScrollBar* vsb = logDisplay.verticalScrollBar();
    vsb->setValue(vsb->maximum());
}

void ProgrammerLogView::updateFileName(){
    fileNameDisplay->setText(ProgrammerLog::get().getLogFileName());
}

void ProgrammerLogView::callSave(){
/*    QFile file(log->getLogFileName());
    QTextStream os (&file);

    log->saveLog(os);

    */
}
