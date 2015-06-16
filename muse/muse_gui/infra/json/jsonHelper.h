#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <jsoncpp/json/json.h>

#include <QString>

#include <fstream>

#include "ServerList.h"
#include "Server.h"

namespace muse {
namespace json {

bool valid(const QString fileName);

Json::Value load(std::ifstream &in);
Json::Value load(QString file);

void save(std::ofstream &out, Json::Value value);
void save(QString file, Json::Value value);

}
}

#endif // JSON_HELPER_H
