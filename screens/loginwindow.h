#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include <QNetworkReply>

#include "mainwindow.h"

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = 0);
    ~LoginWindow();
private slots:
    void on_btnLogin_clicked();
    void getUserInformation();
    void onFinishLoginRequest(QNetworkReply *reply);
    void onFinishGetUser(QNetworkReply *reply);
    void setUpWindow();
    void centerScreen();
    void goToMainScreen(QJsonObject userResponse);
    QString parseResponse(QString message);

private:
    Ui::LoginWindow *ui;

    MainWindow *mainScreen;
    QString authToken;
    QString userId;
};

#endif // LOGINWINDOW_H
