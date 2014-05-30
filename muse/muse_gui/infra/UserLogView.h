#ifndef USER_LOG_VIEW_H
#define USER_LOG_VIEW_H

#include <QTableView>
#include <qcombobox.h>
#include "LogView.h"
#include "UserLog.h"

class UserLogView : public LogView {
    Q_OBJECT
public:
    UserLogView(QWidget *parent = 0);
    
protected slots:
    void updateLog();

    /**
     * @brief Updates the displayed file name of the log file by using the log
     * class' getLogFileName().
     */
    virtual void updateFileName();

private:
    QTableView logDisplay;
    /**
     * @brief Adds additional buttons and features to the toolbar
     * that only exist in the user log view.
     */
    void addWidgetsToToolBar();

    QComboBox loggingLevelSelector;
};

#endif // USER_LOG_VIEW_H
