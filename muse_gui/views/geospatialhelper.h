#ifndef GEOSPATIALHELPER_H
#define GEOSPATIALHELPER_H

#include <QDir>
#include <QImage>

#include <vector>

class GeospatialHelper : public QObject {
    Q_OBJECT

public:
    GeospatialHelper(QDir dir, QObject* parent = nullptr);

private:
    QDir dir;

signals:
    void finished(int zoom, const std::vector<QImage>& images);

public slots:
    void work();
};

#endif // GEOSPATIALHELPER_H
