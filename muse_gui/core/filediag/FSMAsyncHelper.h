#ifndef FSM_ASYNC_HELPER_H
#define FSM_ASYNC_HELPER_H

#include <QRunnable>
#include <QObject>
#include <QModelIndex>

#include "FSHelper.h"

class FSMAsyncHelper : public QObject, public QRunnable {
    Q_OBJECT
public:
    FSMAsyncHelper(const QModelIndex &parent,
                   FSHelper* helper, const FSEntry& dir);
    void run();

signals:
    void entriesLoaded(const QModelIndex& modelIndex);

private:
    const QModelIndex modelIndex;
    FSHelper* const fsHelper;
    const FSEntry& dir;
};

#endif // FSM_ASYNC_HELPER_H
