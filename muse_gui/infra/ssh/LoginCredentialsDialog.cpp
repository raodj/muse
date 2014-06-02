#include "LoginCredentialsDialog.h"
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

LoginCredentialsDialog::LoginCredentialsDialog(QWidget *parent) :
    QDialog(parent), username(this), password(this) {
    // Configure the input fields
    username.setEditable(true);
    // username.setStyleSheet("QComboBox:editable{background:white;}");
    password.setEchoMode(QLineEdit::Password);
    // password.setStyleSheet("QLineEdit{background:white;}");
    // Setup input fields and buttons.
    QDialogButtonBox* btnBox = setupInputFields();
    // Connect buttons to appropriate methods in the base class.
    connect(btnBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
             this, SLOT(reject()));
    connect(btnBox->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
             this, SLOT(accept()));
}

QDialogButtonBox*
LoginCredentialsDialog::setupInputFields(const QString info) {
    // Create labels for input fields
    QLabel *idLabel = new QLabel("User id:", this);
    QLabel *psLabel = new QLabel("Password: ", this);
    // Setup buddies associated with the labels
    idLabel->setBuddy(&username);
    psLabel->setBuddy(&password);
    // Create a grid layout that will contain various information
    QGridLayout *layout = new QGridLayout(this);
    // Setup the label giving user information about the login.
    layout->addWidget(new QLabel(info), 0, 0, 1, 2);
    // Layout user ID and password fields in the form of the grid
    layout->addWidget(idLabel,   1, 0);
    layout->addWidget(&username, 1, 1);
    layout->addWidget(psLabel,   2, 0);
    layout->addWidget(&password, 2, 1);
    // Create a row of buttons at the bottom of the grid layout
    QDialogButtonBox *btnBox = new QDialogButtonBox(this);
    btnBox->addButton(QDialogButtonBox::Ok);
    btnBox->button(QDialogButtonBox::Ok)->setText(tr("Login"));
    btnBox->addButton(QDialogButtonBox::Cancel);
    layout->addWidget(btnBox, 3, 0, 1, 2);
    // Setup the layout for this dialog box
    setLayout(layout);
    // Return the button box for connecting signals back.
    return btnBox;
}

void
LoginCredentialsDialog::setUsername(const QString& userID) {
    int indexPos = username.findText(userID);
    if (indexPos != -1) {
        // Entry exists. So just select it.
        username.setCurrentIndex(indexPos);
    } else {
        // Exactly the same userID does not exist. Add it.
        username.addItem(userID);
        username.setCurrentIndex(username.count() - 1);
    }
}

void
LoginCredentialsDialog::setPassword(const QString& password) {
    this->password.setText(password);
}

void
LoginCredentialsDialog::setUsernamesList(const QStringList &usernames){
    username.addItems(usernames);
}

QString
LoginCredentialsDialog::getUserName() const {
    return username.currentText();
}

QString
LoginCredentialsDialog::getPassword() const {
    return password.text();
}
