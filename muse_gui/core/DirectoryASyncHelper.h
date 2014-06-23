#ifndef DIRECTORY_ASYNC_HELPER_H
#define DIRECTORY_ASYNC_HELPER_H

#include <QRunnable>
#include "SshSocket.h"
#include <QObject>

class DirectoryASyncHelper : public QObject, public QRunnable {
    Q_OBJECT
public:
    DirectoryASyncHelper(const QString methodToCall, const QString& directory,
                         SshSocket* socket);

    /**
     * @brief run Calls on the private methods based on the methodToCall
     * instance variable
     */
    void run();

signals:
    void result(const bool success);
private:
    bool success;
    SshSocket* socket;
    const QString method, directory;

    bool mkdir();
    bool rmdir();

};

#endif
