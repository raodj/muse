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
#include <iostream>
#include <vector>

GeospatialWidget::GeospatialWidget(QWidget *parent, QSize tempSize)
    : QWidget(parent) {
    size = tempSize;
    zoomLevel = 1;

}

GeospatialWidget::~GeospatialWidget(){

}

void GeospatialWidget::paintEvent(QPaintEvent *e) {
    QPainter painter(this);
    this->resize(size);

    std::vector< std::vector<QPixmap> > world;

    /*
    for (int i = 0;; i++){
        if (QPixmap(MUSEGUIApplication::appDir() + "/maps/zoom1/" + QString(i+48) + "_0.png").isNull){
            break;
        }
        std::vector<QPixmap> row();

        for (int j = 0; !QPixmap(MUSEGUIApplication::appDir() + "/maps/zoom1/" + QString(i+48) + "_" + QString(j+48) + ".png").isNull; j++){
            row.push_back(QPixmap(MUSEGUIApplication::appDir() + "/maps/zoom1/" + QString(j+48) + "_" + QString(i+48) + ".png"));
        }
    }

    for (int i = 0; i < 2; i++){
        for (int j = 0; j < 2; j++){
            //QPixmap image(MUSEGUIApplication::appDir() + "/maps/zoom1/" + QString(j+48) + "_" + QString(i+48) + ".png");
            painter.drawPixmap(j*256, i*256, image.width(), image.height(), world[i][j]);
        }
    }
    */

}


#endif
