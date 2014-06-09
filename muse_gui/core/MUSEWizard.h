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

/**
 * @brief The MUSEWizard class The base class for all MUSE Wizard
 * dialogs that will be used throughout the MUSE GUI system.
 */
class MUSEWizard : public QWizard {
public:
    MUSEWizard(QWidget* parent = 0);

    /**
     * @brief addPage Adds a page to the MUSEWizard. This is a MUSE-specific
     * implementation of QWizard's addPage method in that a QLabel for the
     * representation of the page in the side widget is also given as a parameter.
     * @param page The QWizardPage to be added to the MUSEWizard.
     * @param stepName The QLabel textual title of <i>page</i> for display in the
     * side widget step listing
     * @return The pageId of <i>page</i>, as given by QWizard::addPage().
     */
    int addPage(QWizardPage *page, QLabel* stepName);

protected:
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
    QVBoxLayout* mainLayout;
    QWidget sideBar;
};

#endif // MUSE_WIZARD_H
