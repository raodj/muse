#ifndef LOCAL_SERVER_WORKSPACE_H
#define LOCAL_SERVER_WORKSPACE_H

#include "ServerWorkspace.h"

class LocalServerWorkspace : public ServerWorkspace {
public:
    LocalServerWorkspace(const QString& dir);
    ~LocalServerWorkspace();

    void save() override;
    void load() override;
};

#endif // LOCAL_SERVER_WORKSPACE_H
