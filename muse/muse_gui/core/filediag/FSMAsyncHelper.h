#ifndef FSM_ASYNC_HELPER_H
#define FSM_ASYNC_HELPER_H

#include <QRunnable>
#include <QObject>
#include <QModelIndex>

#include "FSHelper.h"

/**
 * @brief The FSMAsyncHelper class a threaded class that assists
 * in the loading of files from the remote server in an
 * asynchronus matter, meaning that the GUI will not be locked
 * while this process is executing.
 */
class FSMAsyncHelper : public QObject, public QRunnable {
    Q_OBJECT
public:
    FSMAsyncHelper(const QModelIndex &parent,
                   FSHelper* helper, const FSEntry& dir);

    /**
     * @brief run The threaded method that begins loading the
     * directories requested by the user in the gui. Once complete,
     * the method emits the entriesLoaded() signal.
     */
    void run();

signals:
    void entriesLoaded(const QModelIndex& modelIndex);

private:
    const QModelIndex modelIndex;
    FSHelper* const fsHelper;
    const FSEntry& dir;
};

#endif // FSM_ASYNC_HELPER_H
