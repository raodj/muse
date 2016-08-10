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
    /**
     * @brief work Retrieves zoom level images and stores them in 'images',
     * and then emits a finished signal with the zoom level and array of images.
     */
    void work();
};

#endif // GEOSPATIALHELPER_H
