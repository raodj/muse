#ifndef FILE_SYSTEM_MODEL_H
#define FILE_SYSTEM_MODEL_H

#include "FSHelper.h"
#include <QFileIconProvider>
#include <QAbstractItemModel>
#include <QStringList>

class FileSystemModel : public QAbstractItemModel {
    Q_OBJECT
public:
    static const int FullPathRole = -255;

    FileSystemModel(QWidget* parent, FSHelper *helper);
    ~FileSystemModel();

    int columnCount(const QModelIndex & = QModelIndex()) const {
        return helper->getColumns();
    }

    QVariant data(const QModelIndex & index,
                  int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex & parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    bool hasChildren(const QModelIndex &parent = QModelIndex()) const;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    bool canFetchMore(const QModelIndex& parent) const;

    QModelIndex toRoot(const QModelIndex& index) const;

    Qt::ItemFlags flags(const QModelIndex& index) const;

    void flushCaches();

    FSHelper* getHelper() const { return helper; }
    FSHelper* operator->() const { return helper; }

    void setHelper(FSHelper *newHelper);

signals:
    void updating(const QModelIndex& parent, bool doneLoading,
                  bool loadingMore);

protected slots:
    void refreshView(const QModelIndex& parent);

protected:
    void startLoading(const QModelIndex &parent, const FSEntry& dir) const;
    mutable QStringList pendingLoads;
    void signalModelChanges(const QModelIndex &parent);
    QIcon getIcon(const FSEntry* entry) const;
private:
    FSHelper *helper;
    QFileIconProvider iconProvider;
};

#endif // FILE_SYSTEM_MODEL_H
