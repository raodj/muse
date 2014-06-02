#ifndef CUSTOM_FILE_DIALOG_H
#define CUSTOM_FILE_DIALOG_H

#include "FileSystemModel.h"
#include "DirFilterProxyModel.h"

#include <QDialog>
#include <QTreeView>
#include <QTableView>
#include <QComboBox>
#include <QToolBar>
#include <QLineEdit>
#include <QButtonGroup>

class CustomFileDialog : public QDialog {
    Q_OBJECT
public:
    explicit CustomFileDialog(QWidget *parent = 0,
                              const QString& dirPath = QString::null);
    void setFileFilters(const QStringList& filters);
    bool setCurrentDir(const QModelIndex &treeIndex);
    QModelIndex getDirIndex(const QString& dirPath = "");
    bool loadSetCurrentDir(const QString& dirPath);

protected:
    QToolBar* createToolBar();
    void configureTreeView();
    void configureTableView();
    QPushButton* addToolButton(QToolBar* toolBar, const QString& iconName,
                               const char* slotMember,
                               const QString& toolTip = "");
    QLayout* configureBottomPanel();
    void showEvent(QShowEvent *se);
    void updatePathComboBoxEntries(const QModelIndex& dir);

protected slots:
    void treeEntrySelected(const QItemSelection& selected,
                           const QItemSelection& deselected);
    void tableEntrySelected(const QItemSelection& selected,
                            const QItemSelection& deselected);
    void tableEntryDoubleClicked(const QModelIndex& index);
    void updateComboBox(QModelIndex dir);
    void dirComboBoxActivated(int index);
    void fileFilterSelected(int index);
    void toParentDir();
    void toHomeDir();
    void selectLocalFS();
    void selectRemoteFS();
    void refresh();
    void updating(const QModelIndex& parent, bool doneLoading, bool busyFlag);

private:
    QTreeView treeView;
    QTableView tableView;
    FileSystemModel fsm;
    DirFilterProxyModel dirFilter;
    DirFilterProxyModel fileFilter;

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
