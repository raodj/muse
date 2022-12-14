#ifndef OVERVIEW_PAGE_H
#define OVERVIEW_PAGE_H

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
#include <QTextEdit>
#include <QFile>

/**
 * @brief The OverviewPage class The opening page for the server wizard.
 * This page informs the user of the necessary information they need to know
 * before adding a server to their workspace directory.
 */
class OverviewPage : public QWizardPage {
public:
    /**
     * @brief OverviewPage One of two constructors for this QWizardPage.
     * The constructor simply loads the text into the QTextEdit to display
     * the information to the user.
     *
     * @param file The QFile that points to the html file that is the source
     * of the text to be displayed on this OverviewPage.
     *
     * @param parent The parent widget this page belongs to.
     */
    OverviewPage(const QFile& file, QWidget *parent = 0);

    /**
     * @brief OverviewPage One of two constructors for this QWizardPage.
     * This constructor takes the text given and places it in the QTextEdit
     * to display the information to the user.
     *
     * @param text The text to be displayed in this page's QTextEdit.
     *
     * @param parent The parent widget this page belongs to
     */
    OverviewPage(const QString& text, QWidget *parent = 0);

    /**
     * @brief setText Set the text to be displayed in this page.
     *
     * This method can be used to set/change the text displayed in this
     * wizard page.
     *
     * @param text The text to be set/displayed in the wizard page. This
     * method simply calls QTextEdit::setText() method to set the text
     * to be displayed. See documentation on QTextEdit::setText() method
     * for additional details on text/html formats.
     */
    void setText(const QString& text);

private:
    /** The text field used to display the overview text in a read-only
     * mode on this wizard page
     */
    QTextEdit overviewText;
};

#endif
