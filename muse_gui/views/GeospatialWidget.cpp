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
#include <vector>

GeospatialWidget::GeospatialWidget(QWidget *parent, QSize tempSize)
    : QWidget(parent), xStart(0), yStart(0), widgetSize(tempSize), zoomLevel(1) {
    loadZoomLevels();
    resize(widgetSize.width() << (zoomLevel-1), widgetSize.height() << (zoomLevel-1));
}

GeospatialWidget::~GeospatialWidget(){

}

void
GeospatialWidget::paintEvent(QPaintEvent *e) {
    std::vector<QPixmap>& map = worldMaps[zoomLevel];

    int size = std::sqrt(map.size());
    int zoom = zoomLevel - 1;

    QPainter painter(this);
    auto tiles = 0;
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int xCoordinate = x * map[2 * x + y].width() << zoom;
            int yCoordinate = y * map[2 * x + y].height() << zoom;
            int differenceThreshold = map[2 * x + y].width();
            if (xCoordinate >= xStart - differenceThreshold && yCoordinate >= yStart - differenceThreshold){
            painter.drawPixmap(xCoordinate, yCoordinate,
                               map[2 * x + y].width() << zoom,
                               map[2 * x + y].height() << zoom,
                               map[2 * x + y]);
            tiles++;
            }
//            painter.drawRect(QRect(x * map[2 * x + y].width() << zoom,
//                                   y * map[2 * x + y].height() << zoom,
//                                   map[2 * x + y].width() << zoom,
//                                   map[2 * x + y].height() << zoom));
        }
    }
    std::cout << "Tiles printed to screen: " << tiles << std::endl;
}
void
GeospatialWidget::loadZoomLevels() {
    QDir dir(MUSEGUIApplication::appDir() + QDir::separator() + "maps");
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QStringList dirs = dir.entryList();

    for (auto& file : dirs) {
        if (file.contains("zoom")) {
            loadZoomLevel(QDir(dir.absoluteFilePath(file)));
        }
    }
}

void
GeospatialWidget::loadZoomLevel(QDir dir){
    dir.setNameFilters(QStringList("*.png"));
    dir.setFilter(QDir::Files);

    QStringList files = dir.entryList();

    int zoom = dir.dirName().split("zoom")[1].toInt();

    for (auto& file : files) {
        worldMaps[zoom].push_back(QPixmap(dir.absoluteFilePath(file)));
    }
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
    }
}

void
GeospatialWidget::zoomOut(){
    if (zoomLevel > 1){
        --zoomLevel;
    }

}

void
GeospatialWidget::xPositionChanged(int x) {
    //std::cout << x << std::endl;
    xStart = x;
}

void
GeospatialWidget::yPositionChanged(int y) {
    //std::cout << y << std::endl;
    yStart = y;
}

#endif
