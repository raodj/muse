#ifndef REMOTE_SERVER_WORKSPACE_H
#define REMOTE_SERVER_WORKSPACE_H

#include "ServerWorkspace.h"
#include "SshChannel.h"
#include "SFtpChannel.h"

class RemoteServerWorkspace : public ServerWorkspace {
public:
    RemoteServerWorkspace(QString dir, const SshChannel& channel);
    QString save() override;
    QString load() override;

private:
    SshChannel& channel;
};

#endif
