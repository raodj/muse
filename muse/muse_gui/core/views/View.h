#ifndef VIEW_H
#define VIEW_H

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
#include <QToolBar>
#include <QMenu>
#include <QVBoxLayout>

/**
 * @brief The View class A generic class for views shown in the MUSE GUI
 * system. At this moment, this class has little to no implementation, but
 * as various view classes are created and common functionality is realized,
 * then the implementation of this class will begin to grow.
 */
class View : public QWidget {
    Q_OBJECT
public:
    /**
     * @brief ~View The destructor for View objects.
     *
     * Currently, the default base class does not have any special clean up
     * operations to perform.  Consequently, the destructor is present
     * to serve as a place holder for future extensions.
     */
    virtual ~View() {}

protected:
    /**
     * @brief View The constructor for creating a view.
     *
     * The constructor creates the default menu entries and buttons
     * associated with the view. This class is an abstract class and is
     * not meant to be directly instantiated.  Consequently, the constructor
     * is protected.
     *
     * @param name The unique name to be associated with this view to ease
     * locating a view within the scope of a given MainWindow.
     *
     * @param parent The logical parent (if any) to be associated with
     * with the view.
     */
    View(const QString& name, QWidget* parent = 0);

    /**
     * @brief addAction Add an action to the view's menu and tool bar.
     *
     * This is a convenience method that must be used to add actions
     * to the view's tool bar.
     *
     * @param action The action to be added to the tool bar. The pointer
     * cannot be NULL and must point to a valid QAction object.
     */
    virtual void addAction(QAction* action);

    /**
     * @brief createDefaultLayout Create a default layout for the view.
     *
     * This method performs the default layout for the view by placing
     * the toolbar at the top of the view and the central widget
     * occupying remainder of the view.
     *
     * @param showToolBar If this flag is set to false, then the toolbar
     * for the view is not displayed.
     *
     * @param centralWidget The main view widget that is layed out to
     * occupy the central part of the view.
     *
     * @return This method returns the box layout used to organize
     * the toolbar (if shown) and the central widget.
     */
    virtual QVBoxLayout* createDefaultLayout(bool showToolBar,
                                             QWidget* centralWidget);

private:
    /**
     * @brief viewToolbar The toolbar that is displayed at the top of a
     * view to provide user with short cuts to common operations associated
     * with the items.  The same set of tools are accessible via the
     * the application-level main menu.
     */
    QToolBar viewToolbar;
};

#endif // VIEW_H
