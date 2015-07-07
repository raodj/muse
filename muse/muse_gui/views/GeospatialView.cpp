#ifndef GEOSPATIAL_VIEW_CPP
#define GEOSPATIAL_VIEW_CPP

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

#include "View.h"

#include "Core.h"
#include "GeospatialView.h"
#include "Workspace.h"
#include "MUSEGUIApplication.h"
#include <iostream>
#include <QHeaderView>
#include <QScrollBar>

// The constant string that identifies the name of this view
const QString GeospatialView::ViewName = "GeospatialView";

GeospatialView::GeospatialView(QWidget *parent) :
                View("GeospatialView", parent)/*, QOpenGL(this)*/ {

    const QString zoomLabels[] = {
        "Zoom 0 (max out)",
        "Zoom 1 (very out)",
        "Zoom 2 (out)",
        "Zoom 3 (slightly out)",
        "Zoom 4 (default)",
        "Zoom 5 (slightly in)",
        "Zoom 6 (in)",
        "Zoom 7 (very in)",
        "Zoom 8 (max in)",
    };

    initializeToolBarButtons();
    scrollArea = new QScrollArea(this);
    scrollArea->lower();
    scrollArea->resize(this->size());
    std::cout << scrollArea->width() << std::endl;
    world = new GeospatialWidget(scrollArea, scrollArea->size());

    world->setMinimumSize(1, 1);
    scrollArea->setWidget(world);

    connect(zoomInButton, SIGNAL(triggered()), this, SLOT(zoomIn()));
    connect(zoomOutButton, SIGNAL(triggered()), this, SLOT(zoomOut()));
    connect(scrollArea->horizontalScrollBar(), SIGNAL(valueChanged(int)),
            world, SLOT(xPositionChanged(int)));
    connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
            world, SLOT(yPositionChanged(int)));
}

void
GeospatialView::initializeToolBarButtons() {

    zoomInButton = new QAction((QIcon(MUSEGUIApplication::appDir() + "/icons/stock_zoom-in.png")),
                                  "Zoom In", 0);
    addAction(zoomInButton);

    zoomOutButton = new QAction((QIcon(MUSEGUIApplication::appDir() + "/icons/stock_zoom-out.png")),
                                  "Zoom Out", 0);
    addAction(zoomOutButton);

}

void
GeospatialView::resizeEvent(QResizeEvent *event) {
    scrollArea->resize(size());
}

void
GeospatialView::zoomIn() {
    world->zoomIn();
}

void
GeospatialView::zoomOut() {
    world->zoomOut();
}


#endif
