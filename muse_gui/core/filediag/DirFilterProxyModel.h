#ifndef DIR_FILTER_PROXY_MODEL_H
#define DIR_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

class DirFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    explicit DirFilterProxyModel(QObject *parent = 0, bool chain = false);
    bool filterAcceptsRow(int row, const QModelIndex &parent) const;
    void enableChain(bool chainFlag);
private:
    bool chain;
};

#endif // DIR_FILTER_PROXY_MODEL_H
