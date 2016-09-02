#include "GeospatialDrawHelper.h"

#include <iostream>
#include <QThread>
#include <QPixmapCache>
#include <QPaintDevice>

GeospatialDrawHelper::GeospatialDrawHelper(QObject *parent, QPainter* p, int x, int y, QImage *img) : QObject(parent)
{
    //QPainter *painter = new QPainter(this);
    this->x = x;
    this->y = y;
    this->image = img;
}

void
GeospatialDrawHelper::doSetup(QThread *cThread){
    connect(cThread, SIGNAL(started()), this, SLOT(work()));

}

void
GeospatialDrawHelper::work(){
    std::cout << "About to paint" << std::endl;
    //painter->drawImage(x, y, *image);
    //painter->drawEllipse(10, 0, 5, 5);
    std::cout << "Painted!" << std::endl;
}
