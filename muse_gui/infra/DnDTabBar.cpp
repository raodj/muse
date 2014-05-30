#ifndef DND_TAB_BAR_CPP
#define DND_TAB_BAR_CPP

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

#include <QApplication>
#include <QtGui>
#include "DnDTabBar.h"
#include "Infrastructure.h"
#include "DnDTabMimeData.h"
#include "DnDTabWidget.h"
#include "Logger.h"

// Named constant (should become a typed constant?)
#define PIXMAP_SIZE 48

DnDTabBar::DnDTabBar(DnDTabWidget *parent) : QTabBar(parent), dndTabWidget(parent) {
    // Set default cursor to indicate tab can be dragged.
    this->setCursor(Qt::OpenHandCursor);
    // Set this bar as a drop target
    this->setAcceptDrops(true);
}

void
DnDTabBar::mousePressEvent(QMouseEvent *me) {
    int tabIdx   = -1;
    // Check if mouse click happend inside a tab in this widget
    if ((me->button() == Qt::LeftButton) &&
        ((tabIdx = tabAt(me->pos())) != -1)) {
        dragStartPoint = me->pos();
    }
    // Let base class handle the mouse clicks for normal operation.
    return QTabBar::mousePressEvent(me);
}

QPixmap
DnDTabBar::getPixmap(QWidget *widget, const QSize size) const {
    QImage snapshot(widget->size(), QImage::Format_ARGB32);
    QPainter painter(&snapshot);
    widget->render(&painter);
    return QPixmap::fromImage(snapshot.scaled(size));
}

void
DnDTabBar::mouseMoveEvent(QMouseEvent *me) {
    // Let base class see the mouse movement (normal operations if any)
    QTabBar::mouseMoveEvent(me);
    // Check and initiate drag-n-drop...
    if ((dragStartPoint.isNull()) ||
        ((me->buttons() & Qt::LeftButton) != Qt::LeftButton)) {
        // The user is not dragging. Invalidate start position and
        // do nothing else.
        dragStartPoint = QPoint();
    } else if ((me->pos() - dragStartPoint).manhattanLength()
               >= QApplication::startDragDistance()) {
        // The user's gesture indicates initation of tab-drag operation
        int tabIdx = tabAt(dragStartPoint);
        DnDTabMimeData *dndData = new DnDTabMimeData(this, tabIdx);
        QDrag *drag = new QDrag(this);
        drag->setMimeData(dndData);
        // Setup pixmap (otherwise Qt generates an error under X11)
        drag->setPixmap(getPixmap(dndTabWidget, QSize(PIXMAP_SIZE, PIXMAP_SIZE)));
        drag->setHotSpot(QPoint(PIXMAP_SIZE / 2, PIXMAP_SIZE / 2));
        // Perform the drag-n-drop operation!
        Qt::DropAction result = drag->exec();
        if (result != Qt::IgnoreAction) {
            // Check and hide this DnDTab if no tabs are left
            dndTabWidget->removeSelf();
        }
    }
}

void
DnDTabBar::dragEnterEvent(QDragEnterEvent *event) {
    event->setAccepted(event->mimeData()->hasFormat(DnDTabMimeData::MimeType));
}

void
DnDTabBar::dragMoveEvent(QDragMoveEvent *event) {
    // Clear out current cue rectangle (may change below)
    cueRect.setRect(0, 0, 0, 0);
    // Process the drag event appropriately
    if (!event->mimeData()->hasFormat(DnDTabMimeData::MimeType)) {
        // Event does not have correct mime type. Ignore it.
        event->ignore();
        return;
    }
    // User is dragging a DnDTabWidget over this bar. Setup cue
    // rectangle appropiately for use in the paint method.
    QWidget* const parent = this->parentWidget();
    const QRect area = parent->rect();
    switch(getDropDirection(event->pos())) {
    case TOP: cueRect = QRect(area.x(), area.y(), area.width(), area.height() / 2);
        break;
    case BOTTOM: cueRect = QRect(area.x(), area.y() + area.height() / 2,
                                 area.width(), area.height() / 2);
        break;
    case LEFT: cueRect = QRect(area.x(), area.y(), area.width() / 2, area.height());
      break;
    case RIGHT: cueRect = QRect(area.x() + area.width() / 2, area.y(),
                                area.width() / 2, area.height());
        break;
    case CENTER:
    default: cueRect = area;
    }
    // Ensure cues are (re)drawn from parent-up
    parent->update();
}

void
DnDTabBar::dropEvent(QDropEvent *event) {
    // Ensure that cue rectangle is eventually cleared from screen.
    cueRect.setRect(0, 0, 0, 0);
    parentWidget()->update();
    // If the event is not of the correct type, ignore it.
    if (!event->mimeData()->hasFormat(DnDTabMimeData::MimeType)) {
        // This event is not of the correct mime type. Ignore it.
        event->setAccepted(false);
        return;
    }
    // The tab was successfully dropped. Suitably handle the gesture.
    const Direction dir = getDropDirection(event->pos());
    const DnDTabMimeData *tabInfo = qobject_cast<const DnDTabMimeData*>(event->mimeData());

    // First extract some information about the tab being moved
    // to streamline the code below.
    DnDTabBar* const srcTab    = tabInfo->getSrcTabBar();
    const int        tabIdx    = tabInfo->getTabIndex();
    QWidget*   const tabWidget = srcTab->dndTabWidget->widget(tabIdx);
    const QColor  tabColor     = srcTab->tabTextColor(tabIdx);
    // Add the widget to current or new DnD Tab.
    this->dndTabWidget->createSplitPane(tabWidget, srcTab->tabText(tabIdx), dir,
                                        srcTab->tabIcon(tabIdx), 50,
                                        srcTab->tabToolTip(tabIdx),
                                        srcTab->tabWhatsThis(tabIdx), &tabColor);
    // Inidcate the drop event was successfully processed
    event->setAccepted(true);
    // Cut a user log for testing purposes
    userLog() << "Tab drag-n-drop completed successfully.";
}

void
DnDTabBar::dragLeaveEvent(QDragLeaveEvent *event) {
    Q_UNUSED(event);
    cueRect.setRect(0, 0, 0, 0);
    parentWidget()->update();
}

DnDTabBar::Direction
DnDTabBar::getDropDirection(const QPoint &mousePos) {
    const int   edgeSize = this->height();
    const QRect region   = this->parentWidget()->rect();
    // Determine direction where tab will be dropped based on
    // the edgeSize and position of the mouse in the region
    if (mousePos.y() < edgeSize) {
        return TOP;
    } else if (mousePos.y() > (region.height() - edgeSize)) {
        return BOTTOM;
    } else if (mousePos.x() < edgeSize) {
        return LEFT;
    } else if (mousePos.x() > (region.width() - edgeSize)) {
        return RIGHT;
    }
    // If the mouse pos is not at the edges then return
    // center as the default direction for the tab.
    return CENTER;
}

void
DnDTabBar::drawCue(QPainter& painter) {
    if (cueRect.isNull()) {
        // No cues to be drawn
        return;
    }
    // Draw cue using the coordinates of cueRect
    painter.setBrush(Qt::Dense7Pattern);
    painter.setPen(Qt::DotLine);
    painter.drawRect(cueRect.adjusted(0, 0, -1, -1));
}

void
DnDTabBar::paintEvent(QPaintEvent *event) {
    // Let base class to necessary painting
    QTabBar::paintEvent(event);
    // Paint rectangle cues (if needed)
    QPainter painter(this);
    drawCue(painter);
}

#endif
