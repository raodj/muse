#include "DirFilterProxyModel.h"
#include "FSEntry.h"

DirFilterProxyModel::DirFilterProxyModel(QObject *parent, bool chain) :
    QSortFilterProxyModel(parent), chain(chain) {
}

bool
DirFilterProxyModel::filterAcceptsRow(int row,
                                      const QModelIndex &parent) const {
    bool accept = false;
    if (parent.isValid() && (parent.internalPointer() != NULL)) {
        const QModelIndex entry = sourceModel()->index(row, 0, parent);
        if (entry.isValid()) {
            const FSEntry *fsEntry = reinterpret_cast<const FSEntry*>(entry.internalPointer());
            accept = (fsEntry->isComputer() || fsEntry->isDir() ||
                      fsEntry->isDrive());
        }
    }
    if (!accept && chain) {
        accept = QSortFilterProxyModel::filterAcceptsRow(row, parent);
    }
    return accept || !parent.isValid();
}

void
DirFilterProxyModel::enableChain(bool chainFlag) {
    chain = chainFlag;
}
