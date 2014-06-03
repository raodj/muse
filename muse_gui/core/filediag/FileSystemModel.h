#ifndef FILE_SYSTEM_MODEL_H
#define FILE_SYSTEM_MODEL_H
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

#include "FSHelper.h"
#include <QFileIconProvider>
#include <QAbstractItemModel>
#include <QStringList>

/**
 * @brief The FileSystemModel class is a model to graphically
 * represent the file system in the CustomFileDialog.
 */
class FileSystemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    static const int FullPathRole = -255;

    FileSystemModel(QWidget* parent, FSHelper *helper);
    ~FileSystemModel();

    /**
     * @brief columnCount Gets the number of columns in the display.
     * @return The number of columns.
     */
    int columnCount(const QModelIndex & = QModelIndex()) const {
        return helper->getColumns();
    }

    /**
     * @brief data Returns the requested data about the file/directory.
     * @param index The index of the item selected in the QModel.
     * @param role The role of this item in the FileSystem.
     * @return A String representing the file path, size, or flags,
     * or a QIcon for its icon in the display.
     */
    QVariant data(const QModelIndex & index,
                  int role = Qt::DisplayRole) const;

    /**
     * @brief index Obtains the index of an entry in the file system.
     * @param row The row that this entry is located in.
     * @param column The column that this entry is located in.
     * @param parent The QModelIndex of the parent entry.
     * @return The QModelIndex of the entry found at row, column.
     */
    QModelIndex index(int row, int column,
                      const QModelIndex & parent = QModelIndex()) const;

    /**
     * @brief parent Obtains the index of the parent directory of
     * the given index of a file or directory.
     * @param index The index of the a file or directory for which
     * the CustomFileDialog needs to find its parent.
     * @return The QModelIndex of the parent of the given file or directory.
     *
     */
    QModelIndex parent(const QModelIndex & index) const;

    /**
     * @brief rowCount Obtains the number of rows of FSEntries
     * that are contained at the given QModelIndex.
     * @param parent The index of the parent.
     * @return The number of rows of FSEntries found at the index.
     */
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    /**
     * @brief hasChildren Returns whether or not the selected directory
     * has children directories or files.
     * @param parent The index of the current directory.
     * @return A boolean indicating whether or not the directory
     * has children.
     */
    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    /**
     * @brief headerData Gets the header data for the specified object.
     * @param section The section of the GUI that this object is found in.
     * @param orientation The orientation of the layout of the objects in the GUI.
     * @param role The object's display role.
     * @return The header that the data represents.
     */
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    /**
     * @brief canFetchMore Returns whether or not more FSEntries can
     * be fetched from the current directory.
     * @param parent The QModelIndex of the current directory.
     * @return A boolean indicating whether or not the gui can fetch
     * more entries.
     */
    bool canFetchMore(const QModelIndex& parent) const;

    /**
     * @brief toRoot Convenience method to convert a given index (typically
     * a directory) to become the "logical" root element for display.
     *
     * This method enables mapping a given index (that was previously
     * obtained from this file system model) to become the "logical" root
     * element.  The "logical" root index can then be used to display
     * the information in another model.  Currently, this method is
     * unused.
     *
     * @param index The index from this model to be mapped as a "logical"
     * root element.
     *
     * @return A model index with the element in index mapped to be the
     * root element.
     */
    QModelIndex toRoot(const QModelIndex& index) const;

    /**
     * @brief flags Returns the set of flags used by the file system.
     * @param index The index of the folder/directory focused on
     * by the current model
     * @return The set of item flags.
     */
    Qt::ItemFlags flags(const QModelIndex& index) const;

    /**
     * @brief flushCaches Flushes the caches, calls
     * beginResetModel(), and endResetModel().
     */
    void flushCaches();

    /**
     * @brief getHelper Gets the FSHelper associated with this FileSystemModel.
     * @return A pointer to the FSHelper associeated with this
     * FileSystemModel.
     */
    FSHelper* getHelper() const { return helper; }

    FSHelper* operator->() const { return helper; }

    /**
     * @brief setHelper Sets a new FSHelper for this
     * FileSystemModel.
     * @param newHelper The new FSHelper.
     */
    void setHelper(FSHelper *newHelper);

signals:
    void updating(const QModelIndex& parent, bool doneLoading,
                  bool loadingMore);

protected slots:

    /**
     * @brief refreshView Refreshes the FileSystem view. Emits
     * the updating() signal.
     * @param parent The current parent directory the user is
     * focused on.
     */
    void refreshView(const QModelIndex& parent);

protected:
    /**
     * @brief startLoading Begins the loading process of a parent directory.
     * @param parent The index of the current parent directory.
     * @param dir The FileSystem entry of the parent directory.
     */
    void startLoading(const QModelIndex &parent, const FSEntry& dir) const;

    mutable QStringList pendingLoads;

    /**
     * @brief signalModelChanges Sends out a series of signals
     * that indicate that changes are taking place within this
     * FileSystemModel.
     *
     * Signals sent are: layoutAboutToBeChanged(), dataChanged(),
     * and layoutChanged().
     *
     * @param parent The entry in the file system model whose information
     * (or underlying information) has changed/been updated by background
     * worker threads.
     */
    void signalModelChanges(const QModelIndex &parent);

    /**
     * @brief getIcon Gets the icon of the entry in the FileSystem.
     * @param entry The entry in the file system.
     * @return The icon used to represent the entry in the
     * CustomFileDialog.
     */
    QIcon getIcon(const FSEntry* entry) const;
private:
    FSHelper *helper;
    QFileIconProvider iconProvider;
};

#endif // FILE_SYSTEM_MODEL_H
