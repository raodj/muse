#ifndef USER_LOG_VIEW_H
#define USER_LOG_VIEW_H

#include <QTableView>
#include <QComboBox>
#include "LogView.h"
#include "UserLog.h"

class UserLogView : public LogView {
    Q_OBJECT
public:
    UserLogView(QWidget *parent = 0);

signals:
    /**
     * @brief saveLog Alerts the GUI that the user log needs to be saved to the file.
     * The int parameter sent by the signal is passed to the UserLog's write().
     *
     * /note
     * This means the QComboBox of log levels MUST remain consistent with the Logger
     * enumerated levels in terms of the order they are listed.
     */
    void saveLog(int);


protected slots:
    void updateLog();

    /**
     * @brief Updates the displayed file name of the log file by using the log
     * class' getLogFileName().
     */
    virtual void updateFileName();

    /**
     * @brief prepareForSave Helper slot/method that allows for the saveLog()
     * signal to be triggered with its necessary parameter of the lowest level
     * of log entry severity to be saved to the log file.
     */
    void prepareForSave();

    /**
     * @brief applyFilterToView Applies the log filter to the GUI, only letting messages
     * of greater or equal importance display on the GUI.
     * @param lowestLevelToShow The lowest severity level of log entry to show.
     */
    void applyFilterToView(const int lowestLevelToShow);

private:
    QTableView logDisplay;
    /**
     * @brief Adds additional buttons and features to the toolbar
     * that only exist in the user log view.
     */
    void addWidgetsToToolBar();

    QComboBox loggingLevelSelector;
    QAction* testSaver;
};

#endif // USER_LOG_VIEW_H
