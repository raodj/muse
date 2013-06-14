#ifndef USER_LOG_VIEW_H
#define USER_LOG_VIEW_H

#include <QTableView>

class UserLogView : public QWidget {
    Q_OBJECT
public:
    explicit UserLogView(QWidget *parent = 0);
    
public slots:
    void updateLog();

private:
    QTableView logDisplay;
};

#endif // USER_LOG_VIEW_H
