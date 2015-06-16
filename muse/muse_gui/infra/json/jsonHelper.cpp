#ifndef JSON_HELPER_CPP
#define JSON_HELPER_CPP

#include <fstream>

#include "jsonHelper.h"

namespace muse {
namespace json {

bool valid(const QString fileName) {
    Json::Value root;
    Json::Reader reader;

    std::ifstream file(fileName.toStdString());

    return reader.parse(file, root);
}

Json::Value load(std::ifstream &in) {
    Json::Value root;
    Json::Reader reader;

    if (!reader.parse(in, root)) {
        throw reader.getFormattedErrorMessages();
    }

    return root;
}

Json::Value load(QString file) {
    std::ifstream in(file.toStdString());
    return load(in);
}

void save(std::ofstream &out, Json::Value value) {
    Json::StyledWriter writer;

    out << writer.write(value);
}

void save(QString file, Json::Value value) {
    std::ofstream out(file.toStdString());
    save(out, value);
}

}
}

#endif
