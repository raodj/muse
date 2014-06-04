#ifndef MUSEWIZARD_H
#define MUSEWIZARD_H

#include <QWizard>

/**
 * @brief The MUSEWizard class The base class for all MUSE Wizard
 * dialogs that will be used throughout the MUSE GUI system.
 */
class MUSEWizard : public QWizard {
public:
    MUSEWizard(QWidget* parent = 0);
};

#endif // MUSEWIZARD_H
