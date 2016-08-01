#ifndef GEOSPATIALVIEW_H
#define GEOSPATIALVIEW_H

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
#include "GeospatialWidget.h"
#include <QTableView>
#include <QScrollArea>

/**
 * @brief The GeospatialView class provides a visual for data received
 * from the redhawk.
 */
class GeospatialView : public View {
    Q_OBJECT

public:
    GeospatialView(QWidget* parent = 0);
    static const QString ViewName;

protected slots:
    /**
     * @brief zoomIn Connects zoomInButton to the GeospatialWidget instance,
     * calling the zoomIn method contained within the widget.
     */
    void zoomIn();

    /**
     * @brief zoomOut Connects zoomInButton to the GeospatialWidget instance,
     * calling the zoomOut method contained within the widget.
     */
    void zoomOut();

    /**
     * @brief zoomOut Passes the size of the scroll area to the GeospatialWidget
     * instance so that the size of the widget may adjust the map accordingly.
     */
    void sendTheSize();

private:
    QAction* zoomInButton;
    QAction* zoomOutButton;

    //This QScrollArea is the container for the GeospatialWidget 'world'.
    QScrollArea *scrollArea;
    //This GeospatialWidget is the instance of the map.
    GeospatialWidget *world;

    /**
     * @brief initializeToolBarButtons Creates toolbar for the map interface,
     * which currently include the zoom-in and zoom-out buttons.
     */
    void initializeToolBarButtons();

    /**
     * @brief resizeEvent This slot checks to see if a change has been made to
     * the size of the GeospatialView and if there is, sends the new size to the
     * GeospatialWidget class to inform the class of the new size of the viewing
     * window.
     *
     * @param event Not used in method but may be used later for precision
     * modifications to the scroll area size.
     */
    void resizeEvent(QResizeEvent* event) override;

};

#endif // GEOSPATIALVIEW_H
