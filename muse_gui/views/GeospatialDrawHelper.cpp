#include "GeospatialDrawHelper.h"

#include <iostream>
#include <thread>
#include <chrono>

#include <QPixmapCache>

GeospatialDrawHelper::GeospatialDrawHelper(/*QObject* parent*/)// : QObject(parent)
{

}

void
GeospatialDrawHelper::work(void (*draw)(int x, int y, QImage *img), int x, int y, QImage *img){
    draw(x, y, img);
}
