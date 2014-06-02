#ifndef REMOTEFSHELPER_H
#define REMOTEFSHELPER_H

#include "FSHelperCommon.h"
#include "SFtpChannel.h"

class RemoteFSHelper : public FSHelperCommon {
    Q_OBJECT
public:
    RemoteFSHelper(SshSocket *ssh, const bool deleteSocket = false);
    ~RemoteFSHelper();

    const FSEntryList* getEntries(const FSEntry& dir) const;
    const FSEntry&     getRoot() const;

    QString  getHomePath()  const;
    QString  getSeparator() const;
    int      getColumns()   const { return 3; }

protected:
    bool populateCache(const FSEntry& parentDir) const;
    void addEntries(const FSEntry& parentDir, SFtpDir& dir, FSEntryList& list) const
    throw (const SshException &);
    void addDir(const FSEntry &parentDir, SFtpDir &dir, FSEntryList& list) const
    throw (const SshException &);
    int  convert(const int sftpFlags) const;

protected slots:
    void moveToThread(QThread* thread, SshSocket *ssh);

signals:
    void needSocket(QThread *thread, SshSocket *ssh);

private:
    SshSocket* const ssh;
    const bool deleteSocket;
    const FSEntry root;
};

#endif // REMOTEFSHELPER_H
