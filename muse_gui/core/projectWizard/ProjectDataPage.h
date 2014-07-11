#ifndef PROJECT_DATA_PAGE_H
#define PROJECT_DATA_PAGE_H

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

#include <QWizardPage>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "RemoteServerSession.h"

/**
 * @brief The ProjectDataPage class A wizard page that prompts the user
 * for the necessary information about the project they are trying to create.
 */
class ProjectDataPage : public QWizardPage {
    Q_OBJECT
public:
    /**
     * @brief ProjectDataPage Creates this wizard page by calling on the
     * private methods to register the fields, add the buttons, and create
     * the connections.
     */
    ProjectDataPage(QWidget* parent = 0);

public slots:
    /**
     * @brief receiveServerSelection Applies the session given by the
     * signal this slot is connected to this Wizard page.
     * @param session The RemoteServerSession to be used for this page.
     */
    void receiveServerSelection(RemoteServerSession* session);

private slots:
    /**
     * @brief browseForSrcFiles Opens a QFileDialog to ask the user for
     * the directory that contains the source files for the project.
     * The page updates the appropriate line edit to display the file path.
     */
    void browseForSrcFiles();
    /**
     * @brief browseForMakeFile Opens a QFileDialog to ask the user for
     * the make file for the project.
     * The page updates the appropriate line edit to display the file path.
     */
    void browseForMakeFile();
    /**
     * @brief browseForExecutable Opens a QFileDialog to ask the user for
     * the project's executable file.
     * The page updates the appropriate line edit to display the file path.
     */
    void browseForExecutable();
    /**
     * @brief browseForOutputDir Opens a QFileDialog to ask the user for
     * the directory to save the output from the project.
     * The page updates the appropriate line edit to display the file path.
     */
    void browseForOutputDir();

private:
    /**
     * @brief connectButtons Connects the browse buttons to their
     * appropriate slots to display a QFileDialog
     */
    void connectButtons();
    /**
     * @brief registerFields Registers all of the QLineEdits
     * as required fields so that the user must enter data in them
     * to advance to the next page.
     */
    void registerFields();

    /**
     * @brief addLineEditAndButtonToPage A convenience method to ease
     * the process of adding the components to this wizard page. This method
     * adds <i>label</i> a row above <i>lineEdit</i> and <i>button</i>, which
     * reside in the same row.
     * @param label A QString explaining what the user should browse for when
     * <i>button</i> is clicked.
     * @param lineEdit A QLineEdit that will display the filepath chosen
     * in the QFileDialog.
     * @param button The button to trigger the appearance of a QFileDialog
     * @param layout The main layout for this wizard
     */
    void addLineEditAndButtonToPage(QString label, QLineEdit* lineEdit, QPushButton* button,
                                    QVBoxLayout* layout);

    QPushButton codeFileBrowse, makeFileBrowse, executableBrowse,
    outputDirBrowse;
    QLineEdit codeFilePath, makeFilePath, executablePath, outputDirPath;
    RemoteServerSession* session;
};

#endif
