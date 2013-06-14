#ifndef DND_TAB_WIDGET_CPP
#define DND_TAB_WIDGET_CPP

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

#include <QLayout>
#include <QtGui>
#include <QMainWindow>
#include "DnDTabWidget.h"
#include "DnDTabMimeData.h"
#include "Logger.h"

DnDTabWidget::DnDTabWidget(QWidget *parent, const QString& name,
                           const bool permanent)
    : QTabWidget(parent), permanent(permanent), tabBar(new DnDTabBar(this)) {
    this->setObjectName(name);
    // Setup a custom tab bar (that shows tabs on top) to intercept
    // mouse events to trigger drag-n-drop events
    this->setTabBar(tabBar);
    // Make the tab's have a close button on them.
    this->setTabsClosable(true);
    // Enable drag-n-drop of tabs
    this->setAcceptDrops(true);
}

void
DnDTabWidget::addTab(QTabWidget *tabWidget, QWidget * child, const QString& name,
                     const QIcon& icon, const QString& toolTip,
                     const QString& helpStr, const QColor* tabColor) {
    // Add child as a tab and set properties for it.
    const int tabIdx = tabWidget->addTab(child, icon, name);
    tabWidget->setTabToolTip(tabIdx, toolTip);
    tabWidget->setTabWhatsThis(tabIdx, helpStr);
    if (tabColor != NULL) {
        tabWidget->tabBar()->setTabTextColor(tabIdx, *tabColor);
    }
}

DnDTabWidget*
DnDTabWidget::createSplitPane(QWidget *child, const QString &name,
                              const DnDTabBar::Direction alignment, const QIcon &icon,
                              const int stretch, const QString& toolTip,
                              const QString& helpStr, const QColor* tabColor) {
    if (alignment == DnDTabBar::CENTER) {
        addTab(this, child, name, icon, toolTip, helpStr, tabColor);
        return this;
    }
    // A new splitter window needs to be created and this DndTabPanel needs to
    // be placed within the newly created split panel. First create the
    // splitter window to be contained by the parent window.
    QSplitter *splitWindow = new QSplitter();
    // Next create a new DndTabWidget to hold the new child window
    DnDTabWidget *sibling = new DnDTabWidget();
    // Add child as a tab and set properties for it.
    addTab(sibling, child, name, icon, toolTip, helpStr, tabColor);

    //Use the helper method to configure the newly created split window.
    setSplitWindow(this->parentWidget(), splitWindow, alignment,
                   this, sibling, stretch);
    // Return newly created dnd panel to the caller.
    return sibling;
}

void
DnDTabWidget::setSplitWindow(QWidget *parent, QSplitter *splitWindow,
                             const DnDTabBar::Direction dir, QWidget *window1,
                             QWidget *window2, const int stretch) {
    // First note the position of the split window in the parent (if parent is
    // a QSplitter) so that the splitWindow can be placed in it is location.
    // Here the logic for adding/moving the QSplitter is different depending
    // on whether the parent is a QSplitter or not.
    int posInParent = -1;
    QList<int> origSizes;
    QSplitter* splitParent = qobject_cast<QSplitter*>(parent);
    if (splitParent != NULL) {
        posInParent = splitParent->indexOf(window1);
        origSizes = splitParent->sizes();
    }

    // Now configure the split window and add child widgets
    const bool isVertical  = (dir == DnDTabBar::TOP)  || (dir == DnDTabBar::BOTTOM);
    const bool isWin2First = (dir == DnDTabBar::TOP)  || (dir == DnDTabBar::LEFT);
    splitWindow->setOrientation(isVertical ? Qt::Vertical : Qt::Horizontal);
    splitWindow->addWidget(isWin2First ? window2 : window1);
    splitWindow->addWidget(isWin2First ? window1 : window2);
    // Set the sizes of the two widgets based on the parent's dimension and dir
    const int dim   = (isVertical ? parent->height() : parent->width());
    const int size1 = dim * stretch / 100;
    QList<int> sizes; // Should use initializer-list instead!
    sizes.append(size1); sizes.append(dim - size1);
    splitWindow->setSizes(sizes);
    // Ensure the size of the child tabs can never become zero
    splitWindow->setChildrenCollapsible(false);

    // Here the logic for adding/moving the QSplitter is different depending
    // on whether the parent is a QSplitter or not.
    if (splitParent == NULL) {
        // The parent is not a splitter. So add it to its layout or set it
        // as central widget if parent is a MainFrame window.
        QMainWindow *mw = qobject_cast<QMainWindow*>(parent);
        if (mw != NULL) {
            // For main window Qt requires to set a central widget.
            mw->setCentralWidget(splitWindow);
        } else if (parent->layout() != NULL)  {
            parent->layout()->addWidget(splitWindow);
        }
    } else  {
        // The parent is a split window.  This is case place splitWindow at
        // the appropriate index and perserve sizes
        splitParent->insertWidget(posInParent, splitWindow);
        splitParent->setSizes(origSizes);
    }
}

void
DnDTabWidget::removeSelf() {
    if ((this->permanent) || (this->count() > 0)) {
        // This tab either contains another page or is permanently
        // present.  Nothing further to be done.
        return;
    }
    // When control drops here, that means we have zero tabs left and
    // it is time to remove ourselves from our parent.
    QWidget *parent = this->parentWidget();
    QSplitter *splitParent = qobject_cast<QSplitter*>(parent);
    if (splitParent != NULL) {
        // Our parent is a QSplitter which needs to be nuked
        // because it will have only 1 component after this window is
        // removed from it. Obtain the sibling in other part of QSplitter
        // (there are always 2 widgets in a QSplitter)
        int myIdx = splitParent->indexOf(this);
        QWidget *sibling = splitParent->widget(1 - myIdx);
        // Now remove the splitParent from its parent.  To do that,
        // first get our parent's parent (our grandparent)
        parent = splitParent->parentWidget();
        QSplitter *grandParent = qobject_cast<QSplitter*>(parent);
        if (grandParent != NULL) {
            grandParent->insertWidget(grandParent->indexOf(splitParent), sibling);
            splitParent->hide();
            splitParent->setParent(NULL);
        } else {
            QMainWindow *mw = qobject_cast<QMainWindow*>(parent);
            if (mw != NULL) {
                // For main window Qt requires to set a central widget.
                mw->setCentralWidget(sibling);
            } else if (parent->layout() != NULL)  {
                parent->layout()->addWidget(sibling);
            }
        }
    }
    // Remove this component and eventually delete it.
    hide();
}

void
DnDTabWidget::dragEnterEvent(QDragEnterEvent *event) {
    tabBar->dragEnterEvent(event);
}

void
DnDTabWidget::dragMoveEvent(QDragMoveEvent *event) {
    tabBar->dragMoveEvent(event);
}

void
DnDTabWidget::dropEvent(QDropEvent *event) {
    tabBar->dropEvent(event);
}

void
DnDTabWidget::dragLeaveEvent(QDragLeaveEvent *event) {
    tabBar->dragLeaveEvent(event);
}

void
DnDTabWidget::paintEvent(QPaintEvent *event) {
    // Let base class to necessary painting
    QTabWidget::paintEvent(event);
    // Paint rectangle cues (if needed)
    QPainter painter(this);
    tabBar->drawCue(painter);
}

#endif
