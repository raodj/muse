#ifndef LOGINCREDENTIALSDIALOG_H
#define LOGINCREDENTIALSDIALOG_H

#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>

class LoginCredentialsDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginCredentialsDialog(QWidget* parent = 0);
    /*!
       * Sets the proposed username, that can come for instance
       * from a shared setting.
       *
       *\param userID the string that represents the current username
       * to display
       */
     void setUsername(const QString& userID);

     /*!
       * Sets the current password to propose to the user for the login.
       *
       * \param password the password to fill into the dialog form
       */
     void setPassword(const QString& password);

     /*!
       * Sets a list of allowed usernames from which the user
       * can pick one if he does not want to directly edit it.
       *
       *\param usernames a list of usernames
       */
     void setUsernamesList(const QStringList& usernames);

     QString getUserName() const;
     QString getPassword() const;

protected:
     QDialogButtonBox* setupInputFields(const QString info = tr("Enter credentials"));

private:
    QComboBox username;
    QLineEdit password;
};

#endif // LOGINCREDENTIALSDIALOG_H
