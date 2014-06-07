#include "SideWidget.h"

SideWidget::SideWidget(QWidget *parent) : QWidget(parent) {


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

    firstRow->addWidget(welcomeCheckmark);
    firstRow->addWidget(welcomeStep);

    secondRow = new QHBoxLayout();
    licenseStep = new QLabel("License");
    licenseStep->setIndent(0);

    secondRow->addWidget(licenseCheckmark);
    secondRow->addWidget(licenseStep);

    thirdRow = new QHBoxLayout();
    finish = new QLabel("Finish");
    finish->setIndent(0);

    thirdRow->addWidget(finishedCheckmark);
    thirdRow->addWidget(finish);

    sideLayout = new QVBoxLayout();
    sideLayout->addLayout(firstRow);
    sideLayout->addLayout(secondRow);
    sideLayout->addLayout(thirdRow);

    setLayout(sideLayout);
}

void
SideWidget::applyCheckMarks(const int pageId) {

    QPixmap box(":/images/16x16/Box.png");
    QPixmap checkBox(":/images/16x16/CheckedBox.png");

    welcomeCheckmark->setPixmap ((pageId > 1) ? checkBox : box);
    licenseCheckmark->setPixmap ((pageId > 2) ? checkBox : box);
    finishedCheckmark->setPixmap ((pageId > 3) ? checkBox : box);


}
