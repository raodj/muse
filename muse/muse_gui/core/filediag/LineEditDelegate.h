#ifndef LINEEDITDELEGATE_H
#define LINEEDITDELEGATE_H

#include <QItemDelegate>

/**
 * @brief The LineEditDelegate class is an extension of the
 * QItemDelegate class that will allow data from the CustomFileDialog
 * to be displayed in a useful way.
 */
class LineEditDelegate : public QItemDelegate {
    Q_OBJECT
public:

    /**
     * @brief LineEditDelegate The constructor for the LineEditDelegate.
     * Currently, the constructor does not have any implementation.
     * @param parent The parent Qt component that this
     * LineEditDelegate belongs to.
     */
    LineEditDelegate(QObject *parent = 0);

    /**
     * @brief createEditor Creates an editor widget for this LineEditDelegate.
     * @param parent The parent widget that this editor will belong to.
     * @param option not currently used.
     * @param index not currently used.
     * @return The editor widget of this LineEditDelegate.
     */
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    /**
     * @brief setEditorData Sets the data to be displayed in the
     * editor.
     * @param editor The editor QWidget that was created by the
     * createEditor() method.
     * @param index The index of the file/directory in CustomFileDialog that
     * is used as the source to set the editor data.
     */
    void setEditorData(QWidget *editor, const QModelIndex &index) const;

    /**
     * @brief setModelData Applies the desired role to an item in the FileSystemModel,
     * which in this case is the Qt::EditRole.
     * @param editor The editor QWidget that was created by the
     * createEditor() method.
     * @param model Pointer to the model that contains the item whose
     * role is to be changed.
     * @param index The QModelIndex of the item whose role is to be set by this
     * method.
     */
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    /**
     * @brief updateEditorGeometry Updates the geometry of editor
     * widget created by createEditor() to be the rectangle geometry.
     * @param editor The pointer to the editor widget.
     * @param option The QStyleOptionViewItem to describe the parameters
     * to use in setting the geometry of the editor.
     * @param index Not currently used.
     */
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif // LINEEDITDELEGATE_H
