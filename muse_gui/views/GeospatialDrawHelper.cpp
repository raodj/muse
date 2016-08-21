#include "GeospatialDrawHelper.h"

#include <iostream>
#include <QThread>
#include <QPixmapCache>

GeospatialDrawHelper::GeospatialDrawHelper(QObject *parent, QPainter* p, int x, int y, QImage *img) : QObject(parent)
{
    this->painter = p;
    this->x = x;
    this->y = y;
    this->image = img;
}

void
GeospatialDrawHelper::doSetup(QThread &cThread){
    connect(&cThread, SIGNAL(started()), this, SLOT(work()));
}

void
GeospatialDrawHelper::work(){
    //painter->drawImage(x, y, *image);
}
