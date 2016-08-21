#ifndef GEOSPATIALDRAWHELPER_H
#define GEOSPATIALDRAWHELPER_H

#include <QDir>
#include <QImage>
#include <QPainter>
#include <vector>

class GeospatialDrawHelper : public QObject {
    Q_OBJECT
public:
    GeospatialDrawHelper(QObject *parent, QPainter* p, int x, int y, QImage *img);
    void doSetup(QThread &cThread);
    QPainter *painter;

private:

    int x;
    int y;
    QImage *image;

public slots:
    void work();


};



#endif // GEOSPATIALDRAWHELPER_H
