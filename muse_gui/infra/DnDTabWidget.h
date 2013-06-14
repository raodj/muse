#ifndef DND_TAB_WIDGET_H
#define DND_TAB_WIDGET_H

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

#include <QTabWidget>
#include <QSplitter>
#include <DnDTabBar.h>

class DnDTabWidget : public QTabWidget {
    Q_OBJECT
public:
    explicit DnDTabWidget(QWidget *parent = 0, const QString& name = "",
                          const bool permanent = false);
    
    virtual ~DnDTabWidget() {}

    DnDTabWidget* createSplitPane(QWidget *child, const QString& name,
                                  const DnDTabBar::Direction alignment,
                                  const QIcon& icon = QIcon(),
                                  const int stretch = 50,
                                  const QString& toolTip = "",
                                  const QString& helpStr = "",
                                  const QColor* tabColor = NULL);
    void removeSelf();

signals:
    
public slots:

protected:
    void setSplitWindow(QWidget* parent, QSplitter *splitWindow,
                        const DnDTabBar::Direction dir, QWidget *window1,
                        QWidget *window2, const int stretch = 50);

    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);
    virtual void dragLeaveEvent(QDragLeaveEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    void addTab(QTabWidget *tabWidget, QWidget* child, const QString& name,
                const QIcon& icon = QIcon(),
                const QString& toolTip = "",
                const QString& tabHelp = "",
                const QColor* tabColor = NULL);
private:
    /**
     * A boolean variable that indicates if this tab is permenant and should
     * not be deleted even if it is empty. This flag is set when the tab is
     * created via the constructor. This value is used by the TabDnDHanlder to
     * decide if this tab should be removed when empty or not. Access's this
     * property via the isPermanent() method.
    */
    const bool permanent;

    /**
     * @brief tabBar The tab bar that displays tabs at the top of the tab area
     * and also handles core drag-and-drop operations. This object is set in
     * the constructor and is never changed during the life time of this object.
     */
    DnDTabBar* const tabBar;
};

#endif // DNDTABWIDGET_H
