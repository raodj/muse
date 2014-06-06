#include "SideWidget.h"

SideWidget::SideWidget(QWidget *parent) : QWidget(parent) {

    //Serves as place holder for the checkbox
    welcomeCheckmark = new QLabel("");
    welcomeCheckmark->setPixmap(QPixmap(":/images/16x16/Box.png"));
    licenseCheckmark = new QLabel("");
    licenseCheckmark->setPixmap(QPixmap(":/images/16x16/Box.png"));
    finishedCheckmark = new QLabel("");
    finishedCheckmark->setPixmap(QPixmap(":/images/16x16/Box.png"));


    firstRow = new QHBoxLayout();
    welcomeStep = new QLabel("Welcome");

    //So that the text can line up
    welcomeStep->setIndent(35);
    QPalette p = welcomeStep->palette();
    //p.setColor(QPalette::Active, QPalette::WindowText, Qt::white);
    //welcomeStep->setPalette(p);

    firstRow->addWidget(welcomeCheckmark);
    firstRow->addWidget(welcomeStep);

    secondRow = new QHBoxLayout();
    licenseStep = new QLabel("License");
    licenseStep->setIndent(0);
    //licenseStep->setPalette(p);
    secondRow->addWidget(licenseCheckmark);
    secondRow->addWidget(licenseStep);

    thirdRow = new QHBoxLayout();
    finish = new QLabel("Finish");
    finish->setIndent(0);
    //finish->setPalette(p);
    thirdRow->addWidget(finishedCheckmark);
    thirdRow->addWidget(finish);

    sideLayout = new QVBoxLayout();
    sideLayout->addLayout(firstRow);
    sideLayout->addLayout(secondRow);
    sideLayout->addLayout(thirdRow);

    setLayout(sideLayout);
}

void
SideWidget::applyCheckMarks(const int id) {

    if (id > 1)
        welcomeCheckmark->setPixmap(QPixmap(":/images/16x16/CheckedBox.png"));
    else welcomeCheckmark->setPixmap(QPixmap(":/images/16x16/Box.png"));

    if (id > 2)
        licenseCheckmark->setPixmap(QPixmap(":/images/16x16/CheckedBox.png"));
    else licenseCheckmark->setPixmap(QPixmap(":/images/16x16/Box.png"));

    if (id > 3)
        finishedCheckmark->setPixmap(QPixmap(":/images/16x16/CheckedBox.png"));
    else finishedCheckmark->setPixmap(QPixmap(":/images/16x16/Box.png"));
}
