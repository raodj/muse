#include "OverviewPage.h"
#include <QFile>
#include <QTextStream>
#include <QVBoxLayout>

OverviewPage::OverviewPage(QWidget* parent) : QWizardPage(parent) {

    //overviewText = new QTextEdit();

    //User can't edit the text in this view port.
    overviewText.setReadOnly(true);

    //load the text from the html file
    QFile page(":/resources/serverOverview.html");
    if(page.open(QFile::ReadOnly)){
        QTextStream input(&page);

        overviewText.setHtml(input.readAll());
        page.close();
    }
    QVBoxLayout* mainLayout = new QVBoxLayout();
    mainLayout->addWidget(&overviewText);
    setLayout(mainLayout);

    //Fixing for Mac display currently.
    setTitle("Overview");
    setSubTitle("Overview of tasks in this wizard");

}
