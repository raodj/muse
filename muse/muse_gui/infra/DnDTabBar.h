#ifndef DND_TAB_BAR_H
#define DND_TAB_BAR_H

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

#include <QTabBar>

// Forward declarations (to keep compiler happy)
class DnDTabWidget;

class DnDTabBar : public QTabBar {
    Q_OBJECT
public:
    /**
     * @brief The Direction enum indicates the various positions at which
     * a widget can be placed in this DnDTabWidget.
     *
     * This enumeration is used primarily by the createSplitPane method
     * in this class to specify the location where a new wiget is to be
     * placed with reference to this DnDTabWidget. If the direction
     * is Direction::CENTER then the widget is added as a tab in this
     * widget. Otherwise the space occupied by this DnDTabWidget is
     * suitably split (using a QSplitter), a new DnDTabWidget is created,
     * and the specified widget is added as a tab to the newly created
     * DnDTabWidget.
     */
    enum Direction {TOP, BOTTOM, LEFT, RIGHT, CENTER};

    explicit DnDTabBar(DnDTabWidget *parent = 0);


    virtual void mousePressEvent(QMouseEvent *me);
    virtual void mouseMoveEvent(QMouseEvent *me);

    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *);

    void drawCue(QPainter &painter);
    void paintEvent(QPaintEvent *event);

signals:
    
public slots:

protected:
    Direction getDropDirection(const QPoint& mousePos);
    QPixmap getPixmap(QWidget *widget, const QSize size = QSize(48, 48)) const;

private:
    QPoint dragStartPoint;
    QRect cueRect;
    DnDTabWidget* const dndTabWidget;
};

#endif // DND_TAB_BAR_H
