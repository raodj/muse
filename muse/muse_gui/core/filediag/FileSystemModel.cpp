#include "FileSystemModel.h"
#include "FSMAsyncHelper.h"

#include <QWidget>
#include <QDateTime>
#include <QThreadPool>

FileSystemModel::FileSystemModel(QWidget *parent, FSHelper *helper) :
    QAbstractItemModel(parent), helper(helper) {
    // Nothing to be done here.
}

FileSystemModel::~FileSystemModel() {
    // Nothing to be done here.
}

QVariant
FileSystemModel::headerData(int section, Qt::Orientation orientation,
                            int role) const {
    if ((role != Qt::DisplayRole) || (orientation != Qt::Horizontal)) {
        return QVariant();
    }
    switch (section) {
    case 0: return QString("Name");
    case 1: return QString("Size");
    case 2: return QString("Last Modified");
    case 3: return QString("Flags");
    }
    return QVariant();
}

Qt::ItemFlags
FileSystemModel::flags(const QModelIndex& index) const {
    // Obtain default values from base class.
    Qt::ItemFlags itemFlags = QAbstractItemModel::flags(index);
    if (index.isValid() && (index.column() == 0)) {
        // The first column that contains files/directory names is editable
        // to permit renaming of directories and files
        itemFlags |= Qt::ItemIsEditable;
    }
    return itemFlags;
}

QVariant
FileSystemModel::data(const QModelIndex& index, int role) const {
    if ((role != Qt::DisplayRole) && (role != FullPathRole) &&
        (role != Qt::EditRole)    && (role != Qt::DecorationRole)){
        return QVariant();
    }
    if (!index.isValid()) {
        return QString("");
    }
    // Obtain the entry from the index.
    const FSEntry *entry = reinterpret_cast<const FSEntry*>(index.internalPointer());
    if (role == FullPathRole) {
        // The user wants the full path. Return it without much ado.
        return entry->getPath();
    } else if (role == Qt::DecorationRole) {
        if (index.column() == 0) {
            return getIcon(entry);
        }
    } else {  // QtDisplayRole or Qt::EditRole
        switch (index.column()) {
        case 0: return helper->getName(*entry);
        case 1: return QString::number(entry->getSize());
        case 2: return QDateTime::fromTime_t(entry->getTimestamp()).toString();
        case 3: return QString::number(entry->getFlags());
        }
    }
    return QString("");
}

QIcon
FileSystemModel::getIcon(const FSEntry *entry) const {
    if (entry->isComputer()) {
        return iconProvider.icon(QFileIconProvider::Computer);
    } else if (entry->isDrive()) {
        return iconProvider.icon(QFileIconProvider::Drive);
    } else if (entry->isDir()) {
        return iconProvider.icon(QFileIconProvider::Folder);
    }
    // Maybe return icon depending on the type of file or file extension?
    return iconProvider.icon(QFileIconProvider::File);
}

QModelIndex
FileSystemModel::index(int row, int column, const QModelIndex &parent) const {
    if (!parent.isValid() || (parent.internalPointer() == NULL)) {
        // Simply return root in this case
        FSEntry &root = const_cast<FSEntry&>(helper->getRoot());
        return createIndex(row, column, &root);
    }
    // For all other indexes obtain necessary data via helper.
    const FSEntry* entry = reinterpret_cast<FSEntry*>(parent.internalPointer());
    Q_ASSERT(entry != NULL);
    if (!helper->isCached(*entry)) {
        // Load data in an asynchronous manner to avoid the GUI from locking.
        startLoading(parent, *entry);
        return QModelIndex();
    }

    const FSEntryList *cacheEntry = helper->getEntries(*entry);
    if ((cacheEntry != NULL) && (cacheEntry->size() > row)) {
        FSEntry &subEntry = const_cast<FSEntry &>(cacheEntry->at(row));
        return createIndex(row, column, &subEntry);
    } else {
        qDebug() << "Unable to find row " << row << " and column "
                 << column << " in " << *entry;
    }
    // Return an invalid model index.
    return QModelIndex();
}

QModelIndex
FileSystemModel::parent(const QModelIndex & index) const {
    if (!index.isValid() || (index.internalPointer() == NULL)) {
        return index; // invalid index
    }
    const FSEntry *entry = reinterpret_cast<const FSEntry*>(index.internalPointer());
    FSEntry& parent = const_cast<FSEntry&>(helper->getParent(*entry));
    if (!parent.isValid()) {
        // Should an exception be thrown here instead?
        return QModelIndex();
    }
    // In order to keep filters and other classes in Qt happy, the
    // row of the parent should be consitent with the value in the
    // grandparent entry (if any).
    const FSEntry& grandParent = helper->getParent(parent);
    int row = 0; // May change in the if below
    if (grandParent.isValid()) {
        const FSEntryList *gpList = helper->getEntries(grandParent);
        // If the dialog is a bit sluggish maybe this call can be
        // replaced by saving index-in-parent value for each FSEntry
        row = gpList->indexOf(parent);
    }
    // Return index with appropriate row.
    return createIndex(row, 0, &parent);
}

int
FileSystemModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) {
        return 1; // We always have 1 entry for computer
    }
    const FSEntry *entry = reinterpret_cast<const FSEntry*>(parent.internalPointer());
    Q_ASSERT ( entry != NULL );
    const bool haveDataInCache   = helper->isCached(*entry);
    if (!haveDataInCache) {
        // Load data in an asynchronous manner to avoid the GUI from locking.
        startLoading(parent, *entry);
        return 1; // For now this is the best guess we have for this entry.
    }
    // Get the entry list as it is in cache.
    const FSEntryList* entryList = helper->getEntries(*entry);
    Q_ASSERT( entryList != NULL );
    return entryList->size();
}

bool
FileSystemModel::hasChildren(const QModelIndex &parent) const {
    if (!parent.isValid() || (parent.internalPointer() == NULL)) {
        return true; // must be root
    }
    const FSEntry* entry = reinterpret_cast<const FSEntry *>(parent.internalPointer());
    return helper->hasChildren(*entry);
}

bool
FileSystemModel::canFetchMore(const QModelIndex& parent) const {
    if (!parent.isValid() || (parent.internalPointer() == NULL)) {
        return true; // must be root
    }
    const FSEntry* entry = reinterpret_cast<const FSEntry *>(parent.internalPointer());
    return !helper->isCached(*entry);
}

QModelIndex
FileSystemModel::toRoot(const QModelIndex &index) const {
    if (!index.isValid() || (index.internalPointer() == NULL)) {
        // Invalid index to begin with.
        return index;
    }
    // Ensure that data for the index has been loaded
    if (rowCount(index) > 0) {
        return createIndex(0, 0, index.internalPointer());
    }
    return index;
}

void
FileSystemModel::signalModelChanges(const QModelIndex& parent) {
    emit layoutAboutToBeChanged();
    emit dataChanged(parent, parent);
    emit layoutChanged();
}

void
FileSystemModel::refreshView(const QModelIndex& parent) {
    const FSEntry *entry = reinterpret_cast<const FSEntry*>(parent.internalPointer());
    Q_ASSERT ( entry != NULL );
    pendingLoads.removeAll(entry->getPath());
    signalModelChanges(parent);
    emit updating(parent, true, !pendingLoads.isEmpty());
}

void
FileSystemModel::startLoading(const QModelIndex &parent,
                              const FSEntry &dir) const {
    if (!pendingLoads.contains(dir.getPath())) {
        pendingLoads.append(dir.getPath());
        FSMAsyncHelper *asyncHelper = new FSMAsyncHelper(parent, helper, dir);
        connect(asyncHelper, SIGNAL(entriesLoaded(QModelIndex)), this,
                SLOT(refreshView(QModelIndex)));
        QThreadPool::globalInstance()->start(asyncHelper);
        emit const_cast<FileSystemModel*>(this)->updating(parent, false, true);
    }
}

void
FileSystemModel::flushCaches() {
    beginResetModel();
    helper->flushCaches();
    endResetModel();
}

void
FileSystemModel::setHelper(FSHelper *newHelper) {
    beginResetModel();
    if (helper != NULL) {
        delete helper;
    }
    helper = newHelper;
    endResetModel();
}
