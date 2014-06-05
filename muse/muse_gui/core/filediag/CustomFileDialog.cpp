#ifndef CUSTOM_FILE_DIALOG_CPP
#define CUSTOM_FILE_DIALOG_CPP

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


#include "CustomFileDialog.h"
#include "LineEditDelegate.h"
#include "SshSocket.h"
#include "LocalFSHelper.h"
#include "RemoteFSHelper.h"
#include "MUSEWorkSpace.h"

#include <QVBoxLayout>
#include <QHeaderView>
#include <QSplitter>
#include <QDebug>
#include <QResizeEvent>
#include <QLabel>
#include <QAction>
#include <QCompleter>
#include <QPushButton>
#include <QProgressDialog>

CustomFileDialog::CustomFileDialog(QWidget *parent, const QString &dirPath)
    : QDialog(parent), treeView(this), tableView(this),
      fsm(this, new LocalFSHelper()), fileFilter(this, true), dirToLoad(dirPath)  {
    // Note: Order of methods called here is important
    // First create toolbar at the top.
    QToolBar* const topBar = createToolBar();
    // Setup the filtere'd source for tree view to show only directories
    dirFilter.setSourceModel(&fsm);
    fileFilter.setSourceModel(&fsm);
    // Setup connections to intercept signals from FileSystemModel and change
    // the cursor to provide visual cues.
    connect(&fsm, SIGNAL(updating(const QModelIndex&, bool, bool)), this,
            SLOT(updating(const QModelIndex&, bool, bool)));
    // Configure properties for the table view
    configureTableView();
    // Configure properties for the tree view
    configureTreeView();
    // Place tree and table view in split panels so user can resize
    // the width of the panels as needed.
    QSplitter *splitPanel = new QSplitter(this);
    splitPanel->addWidget(&treeView);
    splitPanel->addWidget(&tableView);
    splitPanel->setCollapsible(1, false);
    QList<int> sizes;
    sizes << 150 << 250;
    splitPanel->setSizes(sizes);
    splitPanel->setStretchFactor(1, 100);
    // Add plit panel to the main layout
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(topBar, 0);
    layout->addWidget(splitPanel, 1);
    layout->addLayout(configureBottomPanel(), 0);
    this->setLayout(layout);
    this->resize(640, 480);
}

void
CustomFileDialog::showEvent(QShowEvent *se) {
    // Let base class do its job first.
    QDialog::showEvent(se);
    // Load initial folder as needed.
    if (!dirToLoad.isNull()) {
        loadSetCurrentDir(dirToLoad);
    }
}

QPushButton*
CustomFileDialog::addToolButton(QToolBar *toolBar, const QString &iconName,
                                const char *slotMember,
                                const QString& toolTip) {
    const QString iconPath = ":/images/16x16/" + iconName + ".png";
    QPushButton* btn = new QPushButton(QIcon(iconPath), "", toolBar);
    btn->setIconSize(QSize(32, 16));
    btn->setFocusPolicy(Qt::NoFocus);
    btn->setToolTip(toolTip);
    connect(btn, SIGNAL(released()), this, slotMember);
    toolBar->addWidget(btn);
    return btn;
}

QToolBar*
CustomFileDialog::createToolBar() {
    // Create toolbar at the top (it is automaticlaly deleted)
    QToolBar *topBar = new QToolBar("Top Bar", this);
    topBar->setIconSize(QSize(16, 16));
    // Add flat refresh button to the topBar first
    refreshButton = topBar->addAction(QIcon(":/images/16x16/Refresh.png"), "Refresh",
                                      this, SLOT(refresh()));
    topBar->addWidget(new QLabel("Look in:"));
    topBar->addWidget(&dirSelector);
    dirSelector.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    connect(&dirSelector, SIGNAL(activated(int)),
            this, SLOT(dirComboBoxActivated(int)));
    // Add buttons next to the directory selection combo box in tool bar
    toParentBtn =
            addToolButton(topBar, "ParentFolder", SLOT(toParentDir()),
                          "Navigate to the parent folder of current folder");
    addToolButton(topBar, "HomeDirectory", SLOT(toHomeDir()),
                  "Navigate to the home directory");
    addToolButton(topBar, "NewDirectory", SLOT(toParentDir()),
                  "Create a new subdirectory in current directory");
    // Add buttons to change the current view
    topBar->addSeparator();
    QPushButton* const viewGrid =
            addToolButton(topBar, "ViewGrid", SLOT(selectRemoteFS()),
                          "Toggle to a simple view of files");
    viewGrid->setCheckable(true);
    QPushButton* const viewList =
            addToolButton(topBar, "ViewList", SLOT(selectLocalFS()),
                          "Toggle to a detailed list of files");
    viewList->setCheckable(true);
    viewList->setChecked(true);
    // Add view buttons to a button group to ensure only one button
    // is checked/clicked at any given time to display one view at a time
    viewButtons.addButton(viewGrid);
    viewButtons.addButton(viewList);
    return topBar;
}

void
CustomFileDialog::refresh() {
    // Save current directory
    const QString currDir = dirSelector.itemText(dirSelector.count() - 1);
    // Flush out caches and reset the model
    fsm.flushCaches();
    // Start reloading the necessary data.
    loadSetCurrentDir(currDir);
}

void
CustomFileDialog::selectLocalFS() {
    fsm.setHelper(new LocalFSHelper());
    treeView.setCurrentIndex(dirFilter.index(0, 0));
}

void
CustomFileDialog::selectRemoteFS() {
    // Do some ssh testing here for convenience
    QProgressDialog progDiag("Establishing SSH connectivity...",
                             "Cancel", 0, 6, this);
    progDiag.setWindowModality(Qt::WindowModal);
    SshSocket *ssh = NULL;
    try {
        ssh = new SshSocket("Testing SSH connectivity", this, MUSEWorkSpace::getKnownHostsPath());
        if (ssh->connectToHost("redhawk.hpc.miamioh.edu", &progDiag)) {
            fsm.setHelper(new RemoteFSHelper(ssh, true));
            treeView.setCurrentIndex(dirFilter.index(0, 0));
        } else {
            // ssh connection failed.
            qDebug() << "SSH connection to remote host could not be created.";
        }
    } catch (const SshException &exp) {
        SshException::show(exp, this);
        delete ssh;
    }
    progDiag.close();
}

void
CustomFileDialog::updating(const QModelIndex &parent, bool doneLoading,
                           bool busyFlag) {
    // Update cursor and enable refresh-button if background operations are complete
    busyFlag ? setCursor(Qt::BusyCursor) : unsetCursor();
    refreshButton->setEnabled(!busyFlag);
    // Try to set current directory if we are trying to load one.
    if (!dirToLoad.isNull()) {
        loadSetCurrentDir(dirToLoad);
    }
    // Update the options in the File combo box to provide completion suggestions
    const QModelIndex dirIndex = fileFilter.mapFromSource(parent);
    if (doneLoading && (tableView.rootIndex()== dirIndex)) {
        updatePathComboBoxEntries(dirIndex);
    }
}

void
CustomFileDialog::configureTreeView() {
    // fs->setFilter( QDir::AllDirs | QDir::NoDotAndDotDot );
    treeView.setModel(&dirFilter);
    // treeView.header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    treeView.header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    treeView.header()->setStretchLastSection(false);
    treeView.setHeaderHidden(true);
    treeView.hideColumn(1); // We don't show details of directories
    treeView.hideColumn(2); // in tree view. But we could. Not sure
    treeView.hideColumn(3); // if it is useful tho'
    treeView.setUniformRowHeights(true); // Allegedly make view faster
    treeView.setRootIsDecorated(true);   // Need root visible.
    treeView.setSortingEnabled(true);
    treeView.header()->setSortIndicator(0, Qt::AscendingOrder);
    treeView.setEditTriggers(QTreeView::EditKeyPressed |
                             QTreeView::SelectedClicked);
    // Connect tree selection to update list view
    connect(treeView.selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
            this, SLOT(treeEntrySelected(const QItemSelection&, const QItemSelection&)));
    // Setup the root as the currently selected entry (maybe better to do once dialog is visible)
    treeView.setCurrentIndex(dirFilter.mapFromSource(fsm.index(0, 0)));
    // Setup a delegate for renaming files/directories to have better
    // control on the operations. Currently we assume the file name column
    // is column zero. This may not be a good assumption.
    treeView.setItemDelegateForColumn(0, new LineEditDelegate());
}

void
CustomFileDialog::configureTableView() {
    // By default select the root entry
    tableView.setModel(&fileFilter);
    tableView.setRootIndex(fileFilter.index(0, 0));
    tableView.setShowGrid(false);
    tableView.verticalHeader()->setVisible(false);
    tableView.setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView.setWordWrap(false);
    tableView.setSortingEnabled(true);
    tableView.setEditTriggers(QTableView::EditKeyPressed |
                              QTableView::SelectedClicked);
    connect(tableView.selectionModel(),
            SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(tableEntrySelected(const QItemSelection&, const QItemSelection&)));
    connect(&tableView, SIGNAL(doubleClicked(const QModelIndex &)),
            this, SLOT(tableEntryDoubleClicked(const QModelIndex &)));
    // Configure visual stretching behavior to use all horizontal space
    QHeaderView* const header = tableView.horizontalHeader();
    header->setMinimumSectionSize(75);
    header->resizeSection(0, 175);
    header->setStretchLastSection(true);
    // Setup a delegate for renaming files/directories to have better
    // control on the operations. Currently we assume the file name column
    // is column zero. This may not be a good assumption.
    tableView.setItemDelegateForColumn(0, new LineEditDelegate());
}

void
CustomFileDialog::treeEntrySelected(const QItemSelection& selected,
                                    const QItemSelection& deselected) {
    Q_UNUSED(deselected);
    if (selected.isEmpty() || selected[0].isEmpty()) {
        // Nothing is really selected. Nothing further do be done.
        return;
    }
    // Map the selection from DirFilterProxyModel to source FileSystemModel
    QItemSelection srcSelection = dirFilter.mapSelectionToSource(selected);
    // The srcSelection should have one entry for each of the 3 or 4 columns
    // We care only for the first column.
    Q_ASSERT( !srcSelection.isEmpty() && !srcSelection[0].isEmpty() );
    // Convert index to root index
    const QModelIndex treeSelection = srcSelection[0].indexes()[0];
    const QModelIndex dirSelected   = fileFilter.mapFromSource(treeSelection);
    // Have the table display the selected directory contents
    // by setting root index with logical row of 0
    tableView.setRootIndex(dirSelected);
    tableView.setCurrentIndex(dirSelected.child(0, 0));
    tableView.scrollToTop();
    // Update the directory chooser combo box with the currently
    // selected path.
    updateComboBox(selected[0].indexes()[0]);
    // Update the path completion entries as well
    updatePathComboBoxEntries(dirSelected);
}

void
CustomFileDialog::updatePathComboBoxEntries(const QModelIndex &dir) {
    QStringList entryList;
    QModelIndex entry;
    for(int row = 0; ((entry = dir.child(row, 0)).isValid()); row++) {
        entryList.append(entry.data().toString());
    }
    path.hidePopup();
    path.clear();
    path.addItems(entryList);
}

void
CustomFileDialog::updateComboBox(QModelIndex dir) {
    // Clear out the current entries in the combobox.
    dirSelector.clear();
    // Since we are traversing bottom-up but would like entries in the
    // combo-box in top-down order, we insert at index zero. Also,
    // note that dir is in the filtered tree model space.
    while (dir.isValid()) {
        QString path =  dir.data(FileSystemModel::FullPathRole).toString();
        dirSelector.insertItem(0, path);
        dir = dir.parent();
    }
    // Enable/disable button to navigate to parent based on levelToRoot
    if (toParentBtn != NULL) {
        toParentBtn->setEnabled(dirSelector.count() > 1);
    }
}

void
CustomFileDialog::tableEntrySelected(const QItemSelection &selected,
                                     const QItemSelection &deselected) {
    Q_UNUSED(deselected);
    if (selected.isEmpty() || selected[0].isEmpty()) {
        // Nothing is really selected. Nothing further do be done.
        return;
    }
    // Set the name of the file selected in the file entry box.
    const QModelIndex index = tableView.currentIndex();
    const QString     name  = index.data().toString();
    path.setCurrentIndex(path.findText(name));
}

void
CustomFileDialog::tableEntryDoubleClicked(const QModelIndex &index) {
    if (!index.isValid() || (index.internalPointer() == NULL)) {
        // Nothing further to be done.
        return;
    }
    // Extract the FSEntry to see if the double-click was on a directory
    QModelIndex srcIndex = fileFilter.mapToSource(index);
    if (!srcIndex.isValid() || (srcIndex.internalPointer() == NULL)) {
        // Nothing further to be done.
        return;
    }
    const FSEntry* entry =
            reinterpret_cast<const FSEntry *>(srcIndex.internalPointer());
    if (!entry->isDir()) {
        // The selected entry is not a directory. For now nothing else to do
        return;
    }
    // Ok, the entry is a directory. Now change the selection in the tree
    // and that should change the list of the entries in this table. To
    // set index in the tree we need to locate the entry by searching
    // for it (as the filter may have directories in different order)
    const QModelIndex treeIndex = dirFilter.mapFromSource(srcIndex);
    // If we have a valid tree entry was found set it as the current entry
    // which will trigger update of contents in adjacent table view
    if (treeIndex.isValid()) {
        treeView.setExpanded(treeIndex, true);
        treeView.setCurrentIndex(treeIndex);
    }
}

// Slot for handling clicks on button to navigate to parent directory
void
CustomFileDialog::toParentDir() {
    // Use the folder hierarchy in the dirSelector combo box
    // to navigate to the parent folder.
    if (dirSelector.count() > 1) {
        dirComboBoxActivated(dirSelector.count() - 2);
    }
}

// Slot for handling clicks on button to navigate to user's home directory
void
CustomFileDialog::toHomeDir() {
    // Start loading entries for the home path.
    loadSetCurrentDir(fsm->getHomePath());
}

void
CustomFileDialog::dirComboBoxActivated(int index) {
    // The current index refers to the last index in QComboBox. We use
    // that to find the index of the selected index.
    QModelIndex treeIndex = treeView.selectionModel()->selectedIndexes()[0];
    int currIndex = dirSelector.count() - 1; // Since index start at zero
    while (treeIndex.isValid() && (index < currIndex)) {
        treeIndex = treeIndex.parent();
        currIndex--;
    }
    // If the final tree index is valid, then set it up
    if (treeIndex.isValid()) {
        treeView.setCurrentIndex(treeIndex);
    }
}

bool
CustomFileDialog::setCurrentDir(const QModelIndex& treeIndex) {
    if (treeIndex.isValid()) {
        treeView.setExpanded(treeIndex, true);
        treeView.setCurrentIndex(treeIndex);
    }
    // Return true if the tree index is the current one
    return (treeView.currentIndex() == treeIndex);
}

QLayout*
CustomFileDialog::configureBottomPanel() {
    // Setup model for using in the path combo box
    path.setEditable(true);
    path.completer()->setCompletionMode(QCompleter::UnfilteredPopupCompletion);
    // Now create the layout with various components
    QGridLayout *layout = new QGridLayout();
    // Add two rows of labels to the grid in first column
    layout->addWidget(new QLabel("File name: "), 0, 0);
    layout->addWidget(new QLabel("Filter: "),    1, 0);
    // Add fields to the second column and be strechable
    layout->addWidget(&path, 0, 1);
    // path.setStyleSheet("QLineEdit{background:white;}");
    connect(&fileFilters, SIGNAL(activated(int)),
            this, SLOT(fileFilterSelected(int)));
    layout->addWidget(&fileFilters, 1, 1);
    layout->setColumnStretch(1, 1);
    // Add buttons to the last column.
    QPushButton *openBtn =
            new QPushButton(QIcon(":/images/16x16/CheckMark.png"), "Open");
    QPushButton *canelBtn =
            new QPushButton(QIcon(":/images/16x16/CrossMark.png"), "Cancel");
    // Add buttons to the last column.
    layout->addWidget(openBtn,  0, 2);
    layout->addWidget(canelBtn, 1, 2);
    return layout;
}

QModelIndex
CustomFileDialog::getDirIndex(const QString& dirPath) {
    if (dirPath.isEmpty()) {
        // If path is empty by default we return the root index.
        return dirFilter.mapFromSource(fsm.index(0, 0));
    }
    // Find the entry for dirPath using parent of dir path.
    const QString parentPath  = fsm->getParentPath(dirPath);
    QModelIndex   parentIndex = getDirIndex(parentPath);
    // We should have a valid parent path to process further
    if (!parentIndex.isValid()) {
        // Did not find the specified directory. return invalid index
        return QModelIndex();
    }
    // Now that we have a valid parent, search in parent's list for name
    const QString name = fsm->getName(dirPath);
    int row            = 0;
    QModelIndex nameIndex = parentIndex.child(row, 0);
    while (nameIndex.isValid()) {
        if (nameIndex.data() == name) {
            return nameIndex;
        }
        row++;
        nameIndex = parentIndex.child(row, 0);
    }
    // Entry not found
    return QModelIndex();
}

bool
CustomFileDialog::loadSetCurrentDir(const QString &dirPath) {
    QModelIndex index = getDirIndex(dirPath);
    if (index.isValid()) {
        setCurrentDir(index);
        dirToLoad = QString::null;
        return true;
    }
    dirToLoad = dirPath;
    return false;
}

void
CustomFileDialog::setFileFilters(const QStringList &filters) {
    fileFilters.clear();
    fileFilters.addItems(filters);
    fileFilters.setCurrentIndex(0);
    fileFilterSelected(0);
}

void
CustomFileDialog::fileFilterSelected(int index) {
    QRegExp patternExtractor("^(.*)\\(([a-zA-Z0-9_.*? +;#\\-\\[\\]@"\
                             "\\{\\}/!<>\\$%&=^~:\\|]*)\\)$");
    QString filter = fileFilters.itemText(index);
    if (patternExtractor.indexIn(filter) >= 0) {
        QString filterPattern = patternExtractor.cap(2);
        filterPattern = filterPattern.replace("*", "").replace(" ", "|");
        fileFilter.enableChain(true);
        fileFilter.setFilterRegExp(filterPattern);
    } else {
        // Disable filtering.
        fileFilter.enableChain(false);
        fileFilter.setFilterRegExp(".");
    }
}

#endif
