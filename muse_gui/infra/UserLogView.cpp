#include "UserLogView.h"
#include "UserLog.h"
#include <QVBoxLayout>
#include <QHeaderView>

UserLogView::UserLogView(QWidget *parent) :
    LogView(parent, &UserLog::get()), logDisplay(this) {
    // Setup the log display area's grids
    logDisplay.setShowGrid(true);
    logDisplay.setGridStyle(Qt::DotLine);
    // Stretch last column to occupy remaining space
    logDisplay.horizontalHeader()->setStretchLastSection(true);
    // Set the full row to be selected by default
    logDisplay.setSelectionBehavior(QAbstractItemView::SelectRows);
    // Setup the data model for the log display
    logDisplay.setModel(&UserLog::get().getEntries());
    // Organize components in this widget for display.
    QVBoxLayout *layout = new QVBoxLayout();
    //Add the toolbar here.........
    addToToolBar();
    layout->addWidget(this->logToolBar);

    layout->addWidget(&logDisplay, 100);
    this->setLayout(layout);
    // Handle signals about changes in log
    connect(static_cast<const Log*>(&UserLog::get()), SIGNAL(logChanged()),
            this, SLOT(updateLog()));
}

void
UserLogView::updateLog() {
    // Ensure the last row where log was appended is now visible
    const int lastRow      = logDisplay.model()->rowCount() - 1;
    QModelIndex modelIndex = logDisplay.model()->index(lastRow, 0);
    logDisplay.scrollTo(modelIndex, QAbstractItemView::PositionAtBottom);
}

void UserLogView::addToToolBar(){
    logToolBar->addSeparator();

    setLoggingLevel = new QLabel("Set logging level:");
    logToolBar->addWidget(setLoggingLevel);

    loggingLevelSelector = new QComboBox(this);
    loggingLevelSelector->addItem("Testing");

    logToolBar->addWidget(loggingLevelSelector);

}
