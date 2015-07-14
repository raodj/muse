#ifndef REMOTE_SERVER_WORKSPACE_H
#define REMOTE_SERVER_WORKSPACE_H

#include "ServerWorkspace.h"
#include "SshChannel.h"
#include "SFtpChannel.h"
#include "SshSocket.h"

class RemoteServerWorkspace : public ServerWorkspace {
public:
    RemoteServerWorkspace(QString dir, const SshSocket& socket);

    QString save() override;
    QString load() override;

private:
    SshChannel channel;
};

#endif
