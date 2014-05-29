#ifndef USER_LOG_VIEW_H
#define USER_LOG_VIEW_H

#include <QTableView>
#include <qcombobox.h>
#include "LogView.h"
class UserLogView : public LogView {
    Q_OBJECT
public:
    UserLogView(QWidget *parent = 0);
    
public slots:
    void updateLog();

private:
    QTableView logDisplay;
    /**
     * @brief Adds additional buttons and features to the toolbar
     * that only exist in the user log view.
     */
    void addToToolBar();

    QLabel *setLoggingLevel;
    QComboBox *loggingLevelSelector;
};

#endif // USER_LOG_VIEW_H
