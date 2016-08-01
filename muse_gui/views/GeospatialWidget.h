#ifndef GEOSPATIALWIDGET_H
#define GEOSPATIALWIDGET_H

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

#include <QWidget>
#include <QPainter>
#include <QSize>
#include <QScrollArea>
#include <QDir>
#include <QByteArray>
#include <QImage>

#include <unordered_map>
#include <vector>
#include <future>

/**
 * @brief The GeospatialWidget A class which handles the drawing of the world
 * map on the Geospatial View. Responsibilities include loading images into the
 * buffer, reading data points from a file, and generating controls for the
 * zoom-in and zoom-out functionality for navigating the map.
 */

class GeospatialWidget : public QWidget {
    Q_OBJECT

public:
    /**
     * @brief GeospatialWidget The constructor for creating a GeospatialWidget.
     *
     * The constructor creates the default widget, creating the map based on
     * zoom level one image files and loading the rest into memory.
     *
     * @param parent The parent of the GeospatialWidget is passed as to assign
     * the GeospatialWidget to a location within the program. Ideally, this
     * is in the scroll area within the GeospatialView class.
     *
     * @param size The size of the viewing window is passed via this parameter,
     * but the size of the GeospatialWidget may change based on the internal
     * value of the zoom level.
     */
    GeospatialWidget(QWidget *parent, QSize size);

    /**
     * @brief ~GeospatialWidget The destructor for GeospatialWidget objects.
     *
     * Currently, the default base class does not have any special clean up
     * operations to perform.  Consequently, the destructor is present
     * to serve as a place holder for future extensions.
     */
    ~GeospatialWidget();

    /**
     * @brief setZoomLevel This method allows for the direct modification of the
     * GeospatialWidget's zoom level. The current range allowed is from zoom
     * level 1 to 8.
     *
     * @param zoom The passed zoom level is what the method sets for the
     * GeospatialWidget, so long as the value is within a set range.
     */
    void setZoomLevel(int tempZoom);

    /**
     * @brief setZoomLevel Sets the zoom level to the next level.
     *
     * This method utilizes the setZoomLevel method to increment the zoom level
     * by one.
     */
    void zoomIn();

    /**
     * @brief setZoomLevel Sets the zoom level to the previous level.
     *
     * This method utilizes the setZoomLevel method to decrement the zoom level
     * by one.
     */
    void zoomOut();

public slots:
    /**
     * @brief xPositionChanged Monitors the x location of the parent class.
     *
     * This slot tracks the x Position of the parent class. This slot is used
     * within GeospatialWidget to reposition the widget in the event that the
     * parent is moved.
     *
     * @param x The x value for location of the parent class.
     */
    void xPositionChanged(int x);

    /**
     * @brief yPositionChanged Monitors the y location of the parent class.
     *
     * This slot tracks the y Position of the parent class. This slot is used
     * within GeospatialWidget to reposition the widget in the event that the
     * parent is moved.
     *
     * @param y The y value for location of the parent class.
     */
    void yPositionChanged(int y);

    /**
     * @brief getScrollAreaSize Monitors the size of the parent object.
     *
     * This slot tracks the size of the parent in order for the GeospatialWidget
     * to adjust its size accordingly
     *
     * @param scrollSize The size of the parent object.
     */
    void getScrollAreaSize(QSize scrollSize);

    /**
     * @brief retrieveMapImages Grabs images for each zoom level.
     *
     * This method populates arrays for each zoom level with their respective
     * zoom-level tile image.
     *
     * @param zoom The zoom level having its array populated.
     *
     * @param images Vector where map images are to be stored.
     */
    void retrieveMapImages(int zoom, const std::vector<QImage>& images);

protected:
    void paintEvent(QPaintEvent *e);

private:
    QSize widgetSize;
    QSize scrollSize;

    std::vector<std::future<void>> renderImageAsyncCalls;
    std::vector<std::future<void>> loadImagesAsyncCalls;
    //Actual zoom level of the widget
    int zoomLevel;
    //Zoom, derived from zoom level, and used to calculate tile sizes.
    int zoom;
    //Starting x coordinate for drawing
    int xStart;
    //Starting y coordinate for drawing
    int yStart;
    //Array storing the coordinates of each data point
    QByteArray* array;

    //Vector storing the images of all map tiles
    std::unordered_map<int, std::vector<QImage>> worldMaps;
    //Vector storing the status of each zoom level
    std::unordered_map<int, bool> mapLoaded;

    /**
     * @brief loadZoomLevels Loads all zoom levels it finds within the directory.
     *
     * Utilizes multiple threads to load multiple zoom levels simultaneously
     * using QThread.
     *
     */
    void loadZoomLevels();

    /**
     * @brief loadZoomLevel Loads a specified zoom level.
     *
     * Pushes images in the directory to the back of a zoom level's array of
     * tiled images. Before loading all the images, the zoom level is given a
     * status of 'false' for not loaded, which is then changed as all the images
     * are loaded.
     *
     * @param dir Directory of a given zoom level's png images.
     */
    void loadZoomLevel(QDir dir);

    /**
     * @brief automaticResize Resizes the drawing size of tiles
     *
     * Used in the paint method to resize tiles based on the zoom of the image.
     *
     * @param width Width to be modified to the width expected of a certain zoom
     * level.
     *
     * @param height Height to be modified to the height expected of a certain
     * zoom level.
     *
     * @param zoom The zoom necessary to perform the calculation for resizing
     * the tiles within the widget.
     */
    void automaticResize(int width, int height, int zoom);

    /**
     * @brief renderTiles Draws tiles to widget.
     *
     * Using the QPainter instance, renderTiles draws map tiles from the map
     * vector to an image 'img', drawing at the coordinates specified with
     * 'xCoordinate' and 'yCoordinate'. 'x' and 'y' are used to specify the
     * width and height of tiles the full image is (x tiles wide, y tiles tall).
     *
     * @param painter Static painter object used to draw the tiles.
     *
     * @param map Vector of map images to draw.
     *
     * @param x Number tile in a row of tiled images.
     *
     * @param y Number row where tile is stored.
     *
     * @param xCoordinate Starting coordinate on x-axis for drawing.
     *
     * @param yCoordinate Starting coordinate on y-axis for drawing.
     */
    void renderTiles(QPainter *painter, const std::vector<QImage>& map, int x, int y, int xCoordinate, int yCoordinate);

    /**
     * @brief getDataPoints Gets data points from file. (Unfinished method)
     *
     * This method will read data points, for each given zoom level, from file.
     *
     */
    QByteArray* getDataPoints();

    /**
     * @brief drawBuildings Draws images of buildings on the map(Unfinished method)
     *
     * This method will draw buildings on the map on the closer zoom levels,
     * likely to do so procedurally.
     *
     */
    int* drawBuildings();
};


#endif // GEOSPATIALWIDGET_H
