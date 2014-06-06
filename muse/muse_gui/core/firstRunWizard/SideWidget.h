#ifndef SIDEWIDGET_H
#define SIDEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class SideWidget : public QWidget {
public:
    SideWidget(QWidget* parent = 0);
    void applyCheckMarks(const int id);

private:
    QLabel* welcomeStep;
    QLabel* licenseStep;
    QLabel* finish;
    QLabel* welcomeCheckmark;
    QLabel* licenseCheckmark;
    QLabel* finishedCheckmark;
    QVBoxLayout* sideLayout;
    QHBoxLayout* firstRow;
    QHBoxLayout* secondRow;
    QHBoxLayout* thirdRow;
};

#endif // SIDEWIDGET_H
