#ifndef GEOSPATIALDRAWHELPER_H
#define GEOSPATIALDRAWHELPER_H

#include <QDir>
#include <QImage>

#include <vector>

class GeospatialDrawHelper/* : public QObject */{
    //Q_OBJECT

public:
    GeospatialDrawHelper(/*QObject* parent = nullptr*/);

public slots:
    void work(void (*draw)(int x, int y, QImage *img), int x, int y, QImage *img);
};



#endif // GEOSPATIALDRAWHELPER_H
