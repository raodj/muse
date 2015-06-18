#ifndef MUSE_WIZARD_H
#define MUSE_WIZARD_H

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

#include <QWizard>
#include <QLabel>
#include <QVBoxLayout>
#include "OverviewPage.h"

/**
 * @brief The MUSEWizard class The base class for all MUSE Wizard
 * dialogs that will be used throughout the MUSE GUI system.
 */
class MUSEWizard : public QWizard {
public:
    /** Constructor to create wizard with overview loaded from a given file.
     *
     * This constructor can be used to create a MUSE wizard that loads
     * information for its overview from a given HTML file.
     *
     * @param file Path to the file (example: ":/resources/welcome.html")
     * from where the information is to be loaded.
     *
     * @param parent The parent widget (if any).
     */
    MUSEWizard(const QFile& file, QWidget* parent = 0);

    /** Constructor to create wizard with overview information in a given
     * string.
     *
     * This constructor can be used to create a MUSE wizard that uses
     * the data in the given string to create the wizard.
     *
     * @param text A string containing HTML formatted content to be used
     * as the overview description.
     *
     * @param parent The parent widget (if any).
     */
    MUSEWizard(const QString& text, QWidget* parent = 0);

    /**
     * @brief addPage Adds a page to the MUSEWizard.
     *
     * This is a MUSE-specific implementation of QWizard's addPage method in
     * that a QLabel for the representation of the page in the side widget
     * is also given as a parameter.
     *
     * @param page The QWizardPage to be added to the MUSEWizard.
     *
     * @param stepName A short textual page title of <i>page</i> for
     * display in the wizard overview steps listing (to the left of wizard)
     *
     * @param lastPage If this parameter is true then the page is
     * the last page in the wizard. This flag is used to finish GUI-layout of
     * steps on the left-hand-side of the wizard.
     *
     * @return The pageId of <i>page</i>, as given by QWizard::addPage().
     */
    int addPage(QWizardPage *page, const QString &stepName,
                const bool lastPage = false);

protected:
    /** The list of steps in this wizard.  This list is used to display
     * the steps that have been completed and those that are pending.
     */
    QList<QLabel*> steps;

    /**
     * @brief initializePage A simple override of the initializePage()
     * method of the QWizard class that updates the side widget that
     * displays the check list to update the user on their current position.
     *
     * @param id The id of the page to initialize.
     */
    void initializePage(int id);

    /**
     * @brief cleanupPage A simple override of the cleanupPage() method
     * of the QWizard class that updates the side widget that displays
     * the checklist to update the user on their current position.
     * @param id The id of the page to clean up.
     */
    void cleanupPage(int id);

    /** Convenience method to perform the operations required to
     * setup the wizard.  This method is invoked from the constructors
     * to perform the actual setup operation for this class.
     *
     */
    void setup();

private:
    /**
     * @brief addStepToWidget Adds the checkbox and step QLabel being added in
     * addPage() to the side widget of the MUSEWizard to provide the user with
     * a listing of the steps or pages that are in the Wizard.
     */
    void addStepToWidget();

    /**
     * @brief applyCheckMarks Updates the checkboxes in the step listing to
     * reflect where in the MUSEWizard the user is.
     *
     * @param pageId The page that the user will see next.
     */
    void applyCheckMarks(const int pageId);

    /** The main layout for this wizard that contains the size bar and
     * the pages to be displayed.
     */
    QVBoxLayout* mainLayout;

    /** The sidebar widget/panel that is used to display the steps
     * in the wizard to the left-hand-side of the wizard.
     */
    QWidget sideBar;

    /** The default overview page for this wizard that is used to provide
     * the user with a summary information of the tasks to be accomplished
     * by this wizard.
     */
    OverviewPage overviewPage;
};

#endif // MUSE_WIZARD_H
