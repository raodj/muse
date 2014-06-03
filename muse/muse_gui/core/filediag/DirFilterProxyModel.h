#ifndef DIR_FILTER_PROXY_MODEL_H
#define DIR_FILTER_PROXY_MODEL_H

#include <QSortFilterProxyModel>

/**
 * @brief The DirFilterProxyModel class is the filter proxy used
 * in MUSE's CustomFileDialog system to filter the types of files
 * that can be selected in the GUI.
 */
class DirFilterProxyModel : public QSortFilterProxyModel {
    Q_OBJECT
public:
    /**
     * @brief DirFilterProxyModel Constructor for the
     * DirFilterProxyModel. It does not have a specific implementation;
     * it merely calls upon the constructors of its parent class.
     * @param parent The QWidget this class belongs to.
     * @param chain Whether or not the chain is enabled.
     */
    explicit DirFilterProxyModel(QObject *parent = 0, bool chain = false);

    /**
     * @brief filterAcceptsRow Checks to see if the filter accepts
     *
     * @param row
     * @param parent
     * @return
     */
    bool filterAcceptsRow(int row, const QModelIndex &parent) const;
    void enableChain(bool chainFlag);
private:
    bool chain;
};

#endif // DIR_FILTER_PROXY_MODEL_H
