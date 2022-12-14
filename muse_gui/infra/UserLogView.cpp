#include "UserLogView.h"
#include "UserLog.h"
#include <QVBoxLayout>
#include <QHeaderView>

UserLogView::UserLogView(QWidget *parent) : LogView(parent), logDisplay(this) {
    // Setup the log display area's grids
    logDisplay.setShowGrid(true);
    logDisplay.setGridStyle(Qt::DotLine);
    // Stretch last column to occupy remaining space
    logDisplay.horizontalHeader()->setStretchLastSection(true);
    // Set the full row to be selected by default
    logDisplay.setSelectionBehavior(QAbstractItemView::SelectRows);
    // Setup the data model for the log display
    logDisplay.setModel(&UserLog::get().getEntries());
    //Add additional widgets to the toolbar.
    addWidgetsToToolBar();
    // Organize components in this widget for display.
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(logToolBar);
    layout->addWidget(&logDisplay, 100);
    this->setLayout(layout);
    // Handle signals about changes in log
    UserLog& uLog = UserLog::get();
    connect(&uLog, SIGNAL(logChanged()), this, SLOT(updateLog()));
    // Connect signals to change log file name.
    connect(this, SIGNAL(logFileNameChanged(const QString &)),
            &uLog, SLOT(setLogFileName(const QString &)));
    // Setup signal to detect change in log file name (from a different tool/view)
    connect(&uLog, SIGNAL(logFileNameUpdated()),
            this, SLOT(updateFileName()));
    // Connect signal to start/stop saving user logs.
    connect(saveToggleButton, SIGNAL(toggled(bool)),
            &uLog, SLOT(setSaveStatus(bool)));
    // Connect signal to detect change in status to change icon.
    connect(&uLog, SIGNAL(saveStatusChanged(bool)),
            this, SLOT(updateSavePreference(bool)));

    //Connect signal to detect an error in saving the log.
    connect(&uLog, SIGNAL(errorSavingLog(QString)),
            this, SLOT(saveErrorNotification(QString)));

    //Connect signal to append the latest entry to the log file.
    connect(&uLog, SIGNAL(saveNewestEntry()), this, SLOT(prepareForSave()));

    //Connect signal to actually save the file.
    connect(this, SIGNAL(saveLog(int)), &uLog, SLOT(writeLastEntry(int)));

    //Connect signal to change the filter level on the log display
    connect(&loggingLevelSelector, SIGNAL(currentIndexChanged(int)),
            this, SLOT(applyFilterToView(int)));
}

void
UserLogView::updateLog() {

    // Ensure the last row where log was appended is now visible
    const int lastRow      = logDisplay.model()->rowCount() - 1;
    //Check if the latest entry meets the filter requirements.
    //If so, scroll and display it. Otherwise, hide it from view.
    UserLog& uLog = UserLog::get();
    if(uLog.getLevel(lastRow) >= loggingLevelSelector.currentIndex()){
        QModelIndex modelIndex = logDisplay.model()->index(lastRow, 0);
        logDisplay.scrollTo(modelIndex, QAbstractItemView::PositionAtBottom);
    }
    else
        logDisplay.hideRow(lastRow);
}

void
UserLogView::addWidgetsToToolBar() {
    // Set the log-level option in the combo box.
    loggingLevelSelector.addItem("All (Verbose)");
    loggingLevelSelector.addItem("Notice");
    loggingLevelSelector.addItem("Warnings");
    loggingLevelSelector.addItem("Errors");

    // Add a label and log-level selector to the tool bar
    logToolBar->addSeparator();
    logToolBar->addWidget(new QLabel("Set logging level:"));
    logToolBar->addWidget(&loggingLevelSelector);
}

void
UserLogView::updateFileName() {
    fileNameDisplay->setText(UserLog::get().getLogFileName());
}

void
UserLogView::prepareForSave() {
    emit saveLog(loggingLevelSelector.currentIndex());
}

void
UserLogView::applyFilterToView(const int lowestLevelToShow) {
    UserLog& uLog = UserLog::get();
    for(int i = 0; i < logDisplay.model()->rowCount(); i ++) {
        if(uLog.getLevel(i) >= lowestLevelToShow)
            logDisplay.showRow(i);
        else
            logDisplay.hideRow(i);
    }
}
