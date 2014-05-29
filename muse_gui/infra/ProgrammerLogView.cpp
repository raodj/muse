#include "ProgrammerLogView.h"
#include "ProgrammerLog.h"
#include <QVBoxLayout>
#include <QScrollBar>
#include <iostream>

ProgrammerLogView::ProgrammerLogView(QWidget *parent) :
    LogView(parent, &ProgrammerLog::get()), logDisplay(this) {
    logDisplay.setReadOnly(true);
    logDisplay.setWordWrapMode(QTextOption::NoWrap);
    // Organize components in this widget for display.
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(this->logToolBar);
    layout->addWidget(&logDisplay, 100);
    this->setLayout(layout);
    // Handle signals about changes in log
    connect(&ProgrammerLog::get(), SIGNAL(logChanged()), this, SLOT(updateLog()));
    // Setup initial data in programmer log
    updateLog();
    this->updateFileName();
}

void
ProgrammerLogView::updateLog() {
    logDisplay.setText(ProgrammerLog::get().getEntries());
    // Scroll log display to bottom (does not work in X11)
    QScrollBar* vsb = logDisplay.verticalScrollBar();
    vsb->setValue(vsb->maximum());
}
