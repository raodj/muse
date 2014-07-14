#ifndef CUSTOM_FILE_DIALOG_H
#define CUSTOM_FILE_DIALOG_H
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
#include "FileSystemModel.h"
#include "DirFilterProxyModel.h"

#include <QDialog>
#include <QTreeView>
#include <QTableView>
#include <QComboBox>
#include <QToolBar>
#include <QLineEdit>
#include <QButtonGroup>
#include "RemoteServerSession.h"

/**
 * @brief The CustomFileDialog class is an extension of the QFileDialog
 * class in that this class will allow the user to browse files on the
 * remote server, something that the QFileDialog does not allow for.
 */
class CustomFileDialog : public QDialog {
    Q_OBJECT
public:
    /**
     * @brief CustomFileDialog Constructor for the MUSE file dialog,
     * creates a file dialog view essentially from scratch.
     * @param session The RemoteServerSession that will be used if the
     * user wishes to browse the file system on a remote machine.
     * @param parent The parent widget this CustomFileDialog belongs
     * to.
     * @param dirPath The starting directory for the dialog to display
     * upon launching.
     */
    explicit CustomFileDialog(RemoteServerSession* session = NULL, QWidget *parent = 0,
                              const QString& dirPath = QString::null );

    /**
     * @brief setFileFilters Adds the desired file filters to the
     * listing of file filters in the QComboBox.
     * @param filters
     */
    void setFileFilters(const QStringList& filters);

    /**
     * @brief setCurrentDir Sets the current directory in the view.
     * @param treeIndex The index of the tree that is to be the
     * current directory.
     * @return If the tree index is the current index
     */
    bool setCurrentDir(const QModelIndex &treeIndex);

    /**
     * @brief getDirIndex Returns the ModelIndex of the given directory
     * path.
     * @param dirPath The directory path chosen by the user.
     * @return The index of the directory in the Model.
     */
    QModelIndex getDirIndex(const QString& dirPath = "");

    /**
     * @brief loadSetCurrentDir Loads the currently set directory.
     * @param dirPath The currently set directory.
     * @return A boolean indicating whether or not
     * the directory was loaded.
     */
    bool loadSetCurrentDir(const QString& dirPath);

    /**
     * @brief getOpenFileName Gets the path to the file that
     * is selected in this CustomFileDialog.
     * @return File path of selected item.
     */
    QString getOpenFileName();

    /**
     * @brief getOpenFileNames Returns a list of filepaths as selected
     * by the user.
     * @return A QStringList of the selected file paths.
     */
    QStringList getOpenFileNames();

protected:
    /**
     * @brief createToolBar Creates the toolbar for the top of the file
     * dialog.
     * @return A pointer to the created toolbar.
     */
    QToolBar* createToolBar();

    /**
     * @brief configureTreeView Configures the tree view for the file
     * dialog so the user can view the file system as a tree.
     */
    void configureTreeView();

    /**
     * @brief configureTableView Configures the table view for the
     * file dialog so the user can view the file system as a table.
     */
    void configureTableView();

    /**
     * @brief addToolButton Adds a QPushButton to the toolbar.
     *
     * This method provides a convenience API to add a button to the
     * dialog's toolbar. The button is built from the parameters of this
     * method. This method is currently an internal method that is used
     * in the following manner:
     *
     * \code
     *
     * QPushButton *toolBtn =
     * addToolButton(topBar, "ParentFolder", SLOT(toParentDir()),
     *               "Navigate to the parent folder of current folder");
     *
     * \endcode
     *
     * @param toolBar The toolbar the button will be added to once
     * created.
     *
     * @param iconName The alias name of the icon for the PushButton,
     * as specified in the QRC file.
     *
     * @param slotMember The slot method that this button will be
     * connected to. This is the name of the method to which the
     * signals from this buton are dispatched to. An example argument
     * to this parameter is shown in the earlier example.
     *
     * @param toolTip The toolTip to be displayed when the user hovers
     * over the icon or otherwise triggers the tooltip from the gui
     *
     * @return A pointer to the QPushButton that was created.
     *
     */
    QPushButton* addToolButton(QToolBar* toolBar, const QString& iconName,
                               const char* slotMember,
                               const QString& toolTip = "");

    /**
     * @brief configureBottomPanel Sets up the bottom portion of the
     * CustomFileDialog, namely, the file name entry and file fliter
     * selectors, as well as the open and close buttons.
     * @return The layout generated by this method.
     */
    QLayout* configureBottomPanel();

    /**
     * @brief showEvent Shows the selected file and its folders
     * using loadSetCurrentDir().
     *
     * @see loadSetCurrentDir()
     *
     * @param se The show event, which will get passed to the
     * QDialog base class so it can do its job first.
     */
    void showEvent(QShowEvent *se);

    /**
     * @brief updatePathComboBoxEntries Updates the list of directories
     * in the comboBox based on the directory the user has entered.
     * @param dir The directory that the user is in.
     */
    void updatePathComboBoxEntries(const QModelIndex& dir);

protected slots:
    /**
     * @brief treeEntrySelected Sets the focus of the
     * CustomFileDialog to the selected file/folder in the tree view.
     * @param selected The file/folder that was selected.
     * @param deselected The file/folder that was deselected.
     */
    void treeEntrySelected(const QItemSelection& selected,
                           const QItemSelection& deselected);

    /**
     * @brief tableEntrySelected Sets the focus of the
     * CustomFileDialog to the selected file/folder in
     *  the table view.
     * @param selected The file/folder that was selected.
     * @param deselected The file/folder that was deselected.
     */
    void tableEntrySelected(const QItemSelection& selected,
                            const QItemSelection& deselected);

    /**
     * @brief tableEntryDoubleClicked First checks to see if the
     * item that was double-clicked is a directory. If it is, the
     * treeView is updated to show the directory in its expanded
     * form.
     *
     * @param index The index of the item in the table that was
     * double-clicked.
     */
    void tableEntryDoubleClicked(const QModelIndex& index);

    /**
     * @brief updateComboBox Updates the comboBox with new directory entries.
     * @param dir The directory the user is currently focused on.
     */
    void updateComboBox(QModelIndex dir);

    /**
     * @brief dirComboBoxActivated Sets the currentIndex of the
     * treeView if the item selected in the comboBox is valid.
     * @param index The index of the item selected in the
     * comboBox.
     */
    void dirComboBoxActivated(int index);

    /**
     * @brief fileFilterSelected Applies the selected filter, or disables
     * it, as selected by the user.
     * @param index The filter option selected in the QComboBox.
     */
    void fileFilterSelected(int index);

    /**
     * @brief toParentDir Navigates to the parent directory.
     */
    void toParentDir();

    /**
     * @brief toHomeDir Navigates to the user's home directory.
     */
    void toHomeDir();

    /**
     * @brief selectLocalFS Sets up the local file system as the current
     * file system being operated on by this dialog.
     *
     * The custom file dialog has the ability to switch between the file
     * systems on two (or more) machines. This method is used to implement
     * a convenince API (typically via a button) through which the user
     * can quickly switch to navigating the local file system.
     */
    void selectLocalFS();

    /**
     * @brief selectRemoteFS Sets up the file system on the remote
     * machine by first trying to connect to the server.
     */
    void selectRemoteFS();

    /**
     * @brief refresh Refreshes the view in this file dialog.
     */
    void refresh();

    /**
     * @brief updating Provides visual indications to the user that
     * this CustomFileDialog is updating its view.
     *
     * This method is used to intercept asynchronous messages being
     * dispatched by the file system model. The operations of loading
     * data from a file system (typically a remote file system) are
     * performed asychronously (using a pool of background worker
     * threads) as they can be time consuming (at times can take a few
     * seconds). This method intercepts the update notifications provided
     * by the file system model and appropriately updates the corresponding
     * display in the view provided by this class.
     *
     * @param parent The parent QModelIndex that indicates the directory
     * or file entry that has been updated by the file system model.
     * Currently, this method does not use this entry but rather refreshes
     * the full display. In future this parameter could be honoured to
     * selectively refresh the display (so as to reduce flickers/jarring
     * when the whole display updates).  Instead, this method checks to
     * ensure that the current user-selected directory to be loaded
     * (indicated by the dirToLoad instance variable) has been loaded
     * and if so, it updates the list of suggestions.
     *
     * @param doneLoading Whether or not the background loading job
     * is still executing or if it has been completed.
     *
     * @param busyFlag Whether or not the file system is still busy updating
     * other entries (or subdirectories). This method uses this flag to
     * switch between a busy cursor (indicating to the user some background
     * operation is still underway) or a normal cursor.
     */
    void updating(const QModelIndex& parent, bool doneLoading,
                  bool busyFlag);

    /**
     * @brief connectedToRemoteServer Due to the asynchronous manner
     * in which MUSE_GUI interacts with remote servers, this method
     * is needed in the event that the RemoteServerSession has not
     * connected to the server when the user selects to view remote files.
     * The RemoteServerSession notifies the caller via SIGNAL if the connection
     * succeeded or not. With this knowledge, this slot calls selectRemoteFS()
     * again if the connection succeeded.
     * @param connected boolean indication if the session connected to
     * the server.
     */
    void connectedToRemoteServer(bool connected);

private:
    QTreeView treeView;
    QTableView tableView;
    FileSystemModel fsm;
    DirFilterProxyModel dirFilter;
    DirFilterProxyModel fileFilter;
    RemoteServerSession* remoteServer;

    // Widgets in tool bar at top of dialog
    QAction *refreshButton;
    QComboBox dirSelector;
    QPushButton *toParentBtn;
    QButtonGroup viewButtons;

    // Fields in the bottom bialog
    QComboBox fileFilters;
    QComboBox path;

    // Fields for asynchrnously loading and setting current directory
    QString dirToLoad;
};

#endif // CUSTOM_FILE_DIALOG_H
