#ifndef FIRSTRUNWIZARD_H
#define FIRSTRUNWIZARD_H

#include "MUSEWizard.h"

/**
 * @brief The FirstRunWizard class The class that is run to set up
 * the workspace for the user. This wizard only runs the first time
 * the MUSE GUI is run so that the basic workspace environement.
 * FirstRunWizard is an extension of the MUSEWizard class.
 */
class FirstRunWizard : public MUSEWizard {
public:
    FirstRunWizard(QWidget* parent = 0);

private:
    void createTestPages();
};

#endif // FIRSTRUNWIZARD_H
