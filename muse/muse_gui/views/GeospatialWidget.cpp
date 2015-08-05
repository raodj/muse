#ifndef GEOSPATIAL_WIDGET_CPP
#define GEOSPATIAL_WIDGET_CPP

//---------------------------------------------------------------------
//    ___
//   /\__\    This file is part of MUSE    <http://www.muse-tools.org/>
//  /::L_L_
// /:/L:\__\  Miami   University  Simulation  Environment    (MUSE)  is
// \/_/:/  /  free software: you can  redistribute it and/or  modify it
//   /:/  /   under the terms of the GNU  General Public License  (GPL)
//   \/__/    as published  by  the   Free  Software Foundation, either
//            version 3 (GPL v3), or  (at your option) a later version.
//    ___
//   /\__\    MUSE  is distributed in the hope that it will  be useful,
//  /:/ _/_   but   WITHOUT  ANY  WARRANTY;  without  even  the IMPLIED
// /:/_/\__\  WARRANTY of  MERCHANTABILITY  or FITNESS FOR A PARTICULAR
// \:\/:/  /  PURPOSE.
//  \::/  /
//   \/__/    Miami University  and  the MUSE  development team make no
//            representations  or  warranties  about the suitability of
//    ___     the software,  either  express  or implied, including but
//   /\  \    not limited to the implied warranties of merchantability,
//  /::\  \   fitness  for a  particular  purpose, or non-infringement.
// /\:\:\__\  Miami  University and  its affiliates shall not be liable
// \:\:\/__/  for any damages  suffered by the  licensee as a result of
//  \::/  /   using, modifying,  or distributing  this software  or its
//   \/__/    derivatives.
//
//    ___     By using or  copying  this  Software,  Licensee  agree to
//   /\  \    abide  by the intellectual  property laws,  and all other
//  /::\  \   applicable  laws of  the U.S.,  and the terms of the  GNU
// /::\:\__\  General  Public  License  (version 3).  You  should  have
// \:\:\/  /  received a  copy of the  GNU General Public License along
//  \:\/  /   with MUSE.  If not,  you may  download  copies  of GPL V3
//   \/__/    from <http://www.gnu.org/licenses/>.
//
//---------------------------------------------------------------------

#include "GeospatialWidget.h"
#include "MUSEGUIApplication.h"
#include "geospatialhelper.h"

#include <QDir>
#include <QStringList>
#include <QPixmapCache>

#include <iostream>
#include <fstream>
#include <vector>
#include <functional>
#include <cmath>
#include <QFile>
#include <string>
#include <future>

GeospatialWidget::GeospatialWidget(QWidget *parent, QSize tempSize)
    : QWidget(parent), widgetSize{ std::move(tempSize) }, scrollSize{ 50, 50 }, zoomLevel{ 1 },
      zoom{ zoomLevel - 1 }, xStart{ 0 }, yStart{ 0 }
{
    array = getDataPoints();

    loadZoomLevels();
    resize(widgetSize.width() << (zoomLevel-1), widgetSize.height() << (zoomLevel-1));
}

GeospatialWidget::~GeospatialWidget() {

}

void
GeospatialWidget::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e)

    const auto& map = worldMaps[zoomLevel];

    if (map.empty()) {
        return;
    }

    QPainter paint(this);

    automaticResize(map[0].width(), map[0].height(), zoom);

    const float imageSize = map[0].width();

    const float xTilePos = xStart / imageSize;
    const float yTilePos = yStart / imageSize;
    float xTilePosEnd = xTilePos + ((float)scrollSize.width() / imageSize);
    float yTilePosEnd = yTilePos + ((float)scrollSize.height() / imageSize);
    const float xTilePosMax = std::pow(2, zoomLevel);
    const float yTilePosMax = std::pow(2, zoomLevel);

    if (xTilePosEnd >= xTilePosMax) {
        xTilePosEnd = xTilePosMax - 1;
    }

    if (yTilePosEnd >= yTilePosMax) {
        yTilePosEnd = yTilePosMax - 1;
    }

    std::cout << "zoom level: " << zoomLevel << std::endl;
    std::cout << "image size: " << imageSize << std::endl;
    std::cout << "x: " << xStart << " pixel, " << xTilePos << " tile" << std::endl;
    std::cout << "y: " << yStart << " pixel, " << yTilePos << " tile" << std::endl;
    std::cout << "x tiles, start: " << xTilePos << " end: " << xTilePosEnd << std::endl;
    std::cout << "y tiles, start: " << yTilePos << " end: " << yTilePosEnd << std::endl;
    std::cout << "w: " << scrollSize.width() << std::endl;
    std::cout << "h: " << scrollSize.height() << std::endl;

    for (int i = xTilePos; i <= xTilePosEnd; i++){
        for (int j = yTilePos; j <= yTilePosEnd; j++) {
            renderTiles(&paint, map, i, j, i * imageSize, j * imageSize);
        }
    }



    return;

    std::cout << "maps: " << worldMaps.size() << std::endl;

    //const auto& map = worldMaps[zoomLevel];
    const auto size = std::sqrt(map.size());

    if (map.empty()) {
        std::cout << "no images in map: " << zoomLevel << std::endl;
        return;
    }

    automaticResize(map[0].width(), map[0].height(), zoom);
    QPainter painter(this);

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int xCoordinate = x * map[std::pow(2, zoomLevel) * x + y].width() << zoom;
            int yCoordinate = y * map[std::pow(2, zoomLevel) * x + y].height() << zoom;

            int xDifferenceThreshold = ((int)(map[std::pow(2, zoomLevel) * x + y].width())) << zoom;
            int yDifferenceThreshold = ((int)(map[std::pow(2, zoomLevel) * x + y].height())) << zoom;

            if (xCoordinate >= xStart - xDifferenceThreshold && yCoordinate >= yStart - yDifferenceThreshold) {
                if (xCoordinate < xStart + scrollSize.width() && yCoordinate < yStart + scrollSize.height()) {
//                    renderImageAsyncCalls.push_back(std::async(std::launch::async,
//                                                               &GeospatialWidget::renderTiles,
//                                                               this, &painter, map, x, y, xCoordinate, yCoordinate));
                    renderTiles(&painter, map, x, y, xCoordinate, yCoordinate);
                }
            }
        }
    }

    for (int i = 0; i < 10; i++){
        painter.drawText(i*10, 10, (QString)(array->at(i)));
    }
}

void
GeospatialWidget::loadZoomLevels() {
    QDir dir(MUSEGUIApplication::appDir() + QDir::separator() + "maps");
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    std::cout << "application dir: " << dir.absolutePath().toStdString() << std::endl;

    QStringList dirs = dir.entryList();

    qRegisterMetaType<std::vector<QImage>>("std::vector<QImage>");

    for (const auto& d : dirs) {
        if (d.contains("zoom")) {
            QThread* t = new QThread();
            GeospatialHelper* w = new GeospatialHelper(QDir{ dir.absoluteFilePath(d) });

            w->moveToThread(t);

            connect(t, SIGNAL(started()), w, SLOT(work()));
            //connect(t, SIGNAL(finished()), w, SLOT(deleteLater()));

            connect(w, SIGNAL(finished(int, const std::vector<QImage>&)), this, SLOT(retrieveMapImages(int, const std::vector<QImage>&)));

            t->start();

            //loadImagesAsyncCalls.push_back(std::async(std::launch::async, std::bind(&GeospatialWidget::loadZoomLevel, this, QDir{ dir.absoluteFilePath(d) })));
            //loadImagesAsyncCalls.push_back(std::async(std::launch::async, std::bind(load, QDir{ dir.absoluteFilePath(d) }, worldMaps[z])));
            //loadZoomLevel(QDir(dir.absoluteFilePath(d)));
        }
    }
}

void
GeospatialWidget::retrieveMapImages(int zoom, const std::vector<QImage>& images) {
    std::cout << "get images for zoom: " << zoom << " count: " << images.size() << std::endl;
    worldMaps[zoom] = std::move(images);
}

void
GeospatialWidget::loadZoomLevel(QDir dir) {
    dir.setNameFilters(QStringList("*.png"));
    dir.setFilter(QDir::Files);

    QStringList files = dir.entryList();

    int zoom = dir.dirName().split("zoom")[1].toInt();

    mapLoaded[zoom] = false;

    if (zoom > 5) {
        return;
    }

    std::cout << "loading zoom level: " << zoom << " from thread: " << std::this_thread::get_id() << std::endl;
    std::cout << "file count for zoom level: " << zoom << " is " << files.size() << std::endl;
    std::cout << "dir location: " << dir.absolutePath().toStdString() << std::endl;

    int count = 0;

    for (const auto& file : files) {
        //if (dir.absoluteFilePath(file) == "") {
        //    std::cout << "wat" << std::endl;
        //}

        if (count % 10 == 0)
        std::cout << "file: " << count << "/" << files.size() << std::endl;

        //QPixmap t{ dir.absoluteFilePath(file) };
        //QPixmap t{ "/home/kyle/.local/share/MUSE/maps/zoom4/" + QString::number(x) + "_" + QString::number(y) + ".png" };
        worldMaps[zoom].push_back(QImage{ dir.absoluteFilePath(file) });
        count++;
    }

    mapLoaded[zoom] = true;

    std::cout << "done loading zoom level: " << zoom << std::endl;
}

void
GeospatialWidget::setZoomLevel(int tempZoom){
    if (tempZoom > 0 && tempZoom < 9)
        zoomLevel = tempZoom;
}

void
GeospatialWidget::zoomIn(){
    if (zoomLevel < 8 ) {
        ++zoomLevel;
        zoom = zoomLevel - 1;
    }
}

void
GeospatialWidget::zoomOut(){
    if (zoomLevel > 1 ) {
        --zoomLevel;
        zoom = zoomLevel - 1;
    }

}

void
GeospatialWidget::xPositionChanged(int x) {
    xStart = x;
}

void
GeospatialWidget::yPositionChanged(int y) {
    yStart = y;
}

void
GeospatialWidget::getScrollAreaSize(QSize scrollSize) {
    this->scrollSize = scrollSize;
    std::cout << "Received Width: " << scrollSize.width() << " and Height: " << scrollSize.height() << std::endl;
}

void
GeospatialWidget::automaticResize(int tileWidth, int tileHeight, int zoom) {
    this->resize(((int)(pow(2, zoomLevel) * tileWidth)), ((int)(pow(2, zoomLevel) * tileHeight)));
}

void
GeospatialWidget::renderTiles(QPainter* painter, const std::vector<QImage>& map, int x, int y, int xCoordinate, int yCoordinate) {
    const QImage& img = map[std::pow(2, zoomLevel) * x + y];

    if (img.isNull()) {
        return;
    }

//    painter->setPen(Qt::black);
//    painter->drawRect(QRect{ xCoordinate, yCoordinate,
//                             map[std::pow(2, zoomLevel) * x + y].width() << zoom,
//                             map[std::pow(2, zoomLevel) * x + y].height() << zoom});
//    painter->setPen(Qt::white);
//    painter->drawRect(QRect{ xCoordinate + 2, yCoordinate + 2,
//                             (map[std::pow(2, zoomLevel) * x + y].width() << zoom) - 4,
//                             (map[std::pow(2, zoomLevel) * x + y].height() << zoom) - 4 });

    painter->drawImage(xCoordinate, yCoordinate, img.scaled(map[std::pow(2, zoomLevel) * x + y].width(),
                                                            map[std::pow(2, zoomLevel) * x + y].height()));

//    painter->drawImage(xCoordinate, yCoordinate, img.scaled(map[i].width, map[j]));
//
//            painter->drawPixmap(xCoordinate, yCoordinate,
//                               map[std::pow(2, zoomLevel) * x + y].width() << zoom,
//                               map[std::pow(2, zoomLevel) * x + y].height() << zoom,
//                               map[std::pow(2, zoomLevel) * x + y]);

    if (zoomLevel == 8){
        int* tempArray = drawBuildings();
        for (int i = 0; i < 10; i++){
            painter->drawEllipse(i*10, 0, 5, 5);
        }
    }

}

QByteArray*
GeospatialWidget::getDataPoints(){

    QFile file("/home/kyle/Sample_Image_Generation_Files_2/MicroWorld/building_Schools.hapi");
    file.open(QIODevice::ReadOnly);
    QByteArray *temp = new QByteArray(file.readAll());

    if (!file.open(QIODevice::ReadOnly)){

        for (int i = 0; i < temp->size(); i++){
            std::cout << temp->at(i) << std::endl;

        }

    }else{
        std::cout << "File does not exist" << std::endl;
    }

    file.close();

    return temp;
}

int*
GeospatialWidget::drawBuildings(){
    int* array = new int[10];
    for (int i = 0; i < 10; i++){
        array[i] = i;
    }
    return array;
}


#endif
