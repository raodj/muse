#include "LineEditDelegate.h"
#include <QLineEdit>

LineEditDelegate::LineEditDelegate(QObject *parent) : QItemDelegate(parent) {
}

QWidget*
LineEditDelegate::createEditor(QWidget *parent,
    const QStyleOptionViewItem &/* option */,
    const QModelIndex &/* index */) const {
    QLineEdit *editor = new QLineEdit(parent);
    const QColor  bgColor = editor->palette().color(editor->backgroundRole());
    const QString style   = "QLineEdit{background-color: " +
            bgColor.name() + ";}";
    editor->setStyleSheet(style);
    return editor;
}

void
LineEditDelegate::setEditorData(QWidget *editor,
                                const QModelIndex &index) const {
    QString value     = index.model()->data(index, Qt::EditRole).toString();
    QLineEdit *txtBox = static_cast<QLineEdit *>(editor);
    txtBox->setText(value);
}

void
LineEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                              const QModelIndex &index) const {
    QLineEdit *txtBox = static_cast<QLineEdit *>(editor);
    model->setData(index, txtBox->text(), Qt::EditRole);
}

void
LineEditDelegate::updateEditorGeometry(QWidget *editor,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &/* index */) const {
    editor->setGeometry(option.rect);
}

