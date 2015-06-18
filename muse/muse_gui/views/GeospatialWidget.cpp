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
    //painter.drawEllipse(10,10,10,10);
    switch (zoomLevel){
        default:
        case 1:
            for (int i = 0; i < 4; i++){
                for (int j = 0; j < 4; j++){
                    //painter.drawText(MUSEGUIApplication::appDir());
                    //QPixmap image((QString)MUSEGUIApplication::appDir() + "/maps/zoom1/1_0.png");
                    //QPixmap image((Qstring)MUSEGUIApplication::appDir() + "/maps/zoom1/" + j + "_" + i + ".png");
                    //painter.drawPixmap(j*256, i*256, 256,256, image);
                    //painter.drawEllipse(i,j,10,10);
                }
            }
            break;
    }

}


#endif
