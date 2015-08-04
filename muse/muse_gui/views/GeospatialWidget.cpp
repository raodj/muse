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

#include <QDir>
#include <QStringList>

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
    for (int i = 0; i < 8; i++) {
        worldMaps[i] = std::vector<QPixmap>{ };
    }

    loadZoomLevels();
    resize(widgetSize.width() << (zoomLevel-1), widgetSize.height() << (zoomLevel-1));
}

GeospatialWidget::~GeospatialWidget() {

}

void
GeospatialWidget::paintEvent(QPaintEvent *e) {
    Q_UNUSED(e)

    std::cout << "maps: " << worldMaps.size() << std::endl;

    const std::vector<QPixmap>& map = worldMaps[zoomLevel];
    const auto size = std::sqrt(map.size());

    if (map.empty()) {
        std::cout << "no images in map: " << zoomLevel << std::endl;
        return;
    }

    automaticResize(map[0].width(), map[0].height(), zoom);
    QPainter painter(this);

    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            std::async(std::launch::async, &GeospatialWidget::renderTiles, this, &painter, map, x, y);
        }
    }
}
void
GeospatialWidget::loadZoomLevels() {
    QDir dir(MUSEGUIApplication::appDir() + QDir::separator() + "maps");
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    std::cout << "application dir: " << dir.absolutePath().toStdString() << std::endl;

    QStringList dirs = dir.entryList();

    for (auto d : dirs) {
        if (d.contains("zoom")) {
            loadImagesAsyncCalls.push_back(std::async(std::launch::async, &GeospatialWidget::loadZoomLevel, this, QDir{ dir.absoluteFilePath(d) }));
            //loadZoomLevel(QDir(dir.absoluteFilePath(file)));
        }
    }
}

void
GeospatialWidget::loadZoomLevel(QDir dir) {
    dir.setNameFilters(QStringList("*.png"));
    dir.setFilter(QDir::Files);

    QStringList files = dir.entryList();

    int zoom = dir.dirName().split("zoom")[1].toInt();

    std::cout << "loading zoom level: " << zoom << " from thread: " << std::this_thread::get_id() << std::endl;
    std::cout << "file count for zoom level: " << zoom << " is " << files.size() << std::endl;
    std::cout << "dir location: " << dir.absolutePath().toStdString() << std::endl;

    for (auto& file : files) {
        worldMaps[zoom].push_back(QPixmap(dir.absoluteFilePath(file)));
    }

    std::cout << "done loading zoom level: " << zoom << std::endl;
}

void
GeospatialWidget::setZoomLevel(int tempZoom){
    if (tempZoom > 0 && tempZoom < 9)
        zoomLevel = tempZoom;
}

void
GeospatialWidget::zoomIn(){
    if (zoomLevel < 8){
        ++zoomLevel;
        zoom = zoomLevel - 1;
        //map = worldMaps[zoomLevel];
    }
}

void
GeospatialWidget::zoomOut(){
    if (zoomLevel > 1){
        --zoomLevel;
        zoom = zoomLevel - 1;
        //map  = worldMaps[zoomLevel];
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
    this->resize(((int)(pow(2, zoomLevel) * tileWidth)) << zoom, ((int)(pow(2, zoomLevel) * tileHeight)) << zoom);
}

void
GeospatialWidget::renderTiles(QPainter* painter, const std::vector<QPixmap>& map, int x, int y) {
    int xCoordinate = x * map[std::pow(2, zoomLevel) * x + y].width() << zoom;
    int yCoordinate = y * map[std::pow(2, zoomLevel) * x + y].height() << zoom;

    int xDifferenceThreshold = ((int)(map[std::pow(2, zoomLevel) * x + y].width())) << zoom;
    int yDifferenceThreshold = ((int)(map[std::pow(2, zoomLevel) * x + y].height())) << zoom;

    if (xCoordinate >= xStart - xDifferenceThreshold && yCoordinate >= yStart - yDifferenceThreshold){
        if (xCoordinate < xStart + scrollSize.width() && yCoordinate < yStart + scrollSize.height()){

            painter->drawPixmap(xCoordinate, yCoordinate,
                               map[std::pow(2, zoomLevel) * x + y].width() << zoom,
                               map[std::pow(2, zoomLevel) * x + y].height() << zoom,
                               map[std::pow(2, zoomLevel) * x + y]);
        }
    }

    if (zoomLevel == 8){
        int* tempArray = drawBuildings();
        for (int i = 0; i < 10; i++){
            painter->drawEllipse(i*10, 0, 5, 5);
        }
    }

}

QByteArray*
GeospatialWidget::getDataPoints(){

    QFile file("home/kyle/Sample Image Generation Files 2/MicroWorld/building_Schools.hapi");
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
