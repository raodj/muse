#ifndef FILE_SYSTEM_MODEL_H
#define FILE_SYSTEM_MODEL_H

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
     * @brief headerData
     * @param section
     * @param orientation
     * @param role
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
     * @param parent
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
