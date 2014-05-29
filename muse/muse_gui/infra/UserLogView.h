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
    
public slots:
    void updateLog();

    /**
     * @brief Updates the displayed file name of the log file by using the log
     * class' getLogFileName().
     */
    void updateFileName();

    /**
     * @brief Tells this UserLogView to call the save method of the log it belongs
     * to. This must be done in order to observe the desired Log filter as set in
     * the GUI by the user.
     */
    void callSave();

private:
    QTableView logDisplay;
    /**
     * @brief Adds additional buttons and features to the toolbar
     * that only exist in the user log view.
     */
    void addToToolBar();

    QLabel *setLoggingLevel;
    QComboBox *loggingLevelSelector;
    UserLog *log;
};

#endif // USER_LOG_VIEW_H
