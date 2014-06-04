#include "FirstRunWizard.h"

//For testing purposes...
#include <QWizardPage>
FirstRunWizard::FirstRunWizard(QWidget *parent)
    : MUSEWizard(parent) {

    setWindowTitle("Set up workspace");
    createTestPages();
}

void
FirstRunWizard::createTestPages() {

    QWizardPage* testOne = new QWizardPage();
    testOne->setTitle("WELCOME");
    testOne->setSubTitle("This is merely a test page. Well done");
    addPage(testOne);
}
