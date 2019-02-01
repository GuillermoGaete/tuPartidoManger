#include "screens/loginwindow.h"
#include "screens/mainwindow.h"
#include "ui_loginwindow.h"
#include "../services/networkmanager.h"

#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QMessageBox>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QNetworkConfigurationManager>
#include <QDesktopWidget>
#include <QStatusBar>

#define LOGIN_ENDPOINT "https://www.googleapis.com/identitytoolkit/v3/relyingparty/verifyPassword?key=AIzaSyBDJc9LXsawBr08jv7J7UT_3yONjA9yo5A"
#define USER_ENDPOINT_BASE "https://tu-partido-video.firebaseio.com/"

#define TEST_EMAIL "morumbi@tupartido.com"
#define TEST_PASSWORD "123456"

LoginWindow::LoginWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoginWindow)
{
    ui->setupUi(this);
    this->centerScreen();
    this->setUpWindow();
#ifdef QT_DEBUG
    //In order to test we complete email and password in debug mode
    this->ui->edtEmail->setText(TEST_EMAIL);
    this->ui->edtPassword->setText(TEST_PASSWORD);
#endif
}

LoginWindow::~LoginWindow()
{
    delete ui;
}

void LoginWindow::centerScreen(){
    QDesktopWidget wid;

    int screenWidth = wid.screen()->width();
    int screenHeight = wid.screen()->height();

    this->setGeometry((screenWidth/2)-(this->width()/2),(screenHeight/2)-(this->height()/2),this->width(),this->height());
}

void LoginWindow::on_btnLogin_clicked()
{
    this->ui->btnLogin->setEnabled(false);
    this->ui->edtEmail->setEnabled(false);
    this->ui->edtPassword->setEnabled(false);

    this->ui->statusbar->showMessage("Iniciando sesion...");
    this->ui->lblErrorMessage->setVisible(false);


    QUrl loginUrl = QUrl(LOGIN_ENDPOINT);

    QJsonObject body;

    body.insert("email", this->ui->edtEmail->text());
    body.insert("password", this->ui->edtPassword->text());
    body.insert("returnSecureToken", true);

    QNetworkAccessManager * networkManager = new QNetworkAccessManager(this);

    QNetworkRequest request(loginUrl);

    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    connect(networkManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(onFinishLoginRequest(QNetworkReply*)));
    connect(networkManager,SIGNAL(finished(QNetworkReply*)),networkManager,SLOT(deleteLater()));

    networkManager->post(request, QJsonDocument(body).toJson());
}
void LoginWindow::onFinishLoginRequest(QNetworkReply *reply)
{
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    int status = statusCode.toInt();
    QByteArray bytesResponse = reply->readAll();
    QJsonObject jsonResponse= QJsonDocument::fromJson(bytesResponse).object();

    switch (status) {
    case 200:
            qDebug()<<"Login ok\n";
            this->authToken = jsonResponse.value("idToken").toString();
            this->userId = jsonResponse.value("localId").toString();
            this->getUserInformation();
        break;
    case 400:
            qDebug()<<"Login error\n";
            QString error = jsonResponse.value("error").toObject().value("message").toString();
            this->ui->lblErrorMessage->setText(this->parseResponse(error));
            this->ui->lblErrorMessage->setVisible(true);
            this->ui->btnLogin->setEnabled(true);
            this->ui->edtEmail->setEnabled(true);
            this->ui->edtPassword->setEnabled(true);
            qDebug()<<"Error response"<<bytesResponse;
        break;
    }

}

QString LoginWindow::parseResponse(QString message){
    QString parsedMessage = message;
    if (message=="INVALID_EMAIL"){
        parsedMessage="Correo inválido.";
    }
    if (message=="INVALID_PASSWORD"){
        parsedMessage="Contraseña inválida.";
    }
    if (message=="MISSING_PASSWORD"){
        parsedMessage="Falta Contraseña.";
    }
    if (message=="EMAIL_NOT_FOUND"){
        parsedMessage="No existe el usuario.";
    }
    if (message=="TOO_MANY_ATTEMPTS_TRY_LATER : Too many unsuccessful login attempts.  Please include reCaptcha verification or try again later"){
        parsedMessage="Intente más tarde.";
    }
    return parsedMessage;
}


void LoginWindow::setUpWindow(){
    this->ui->btnLogin->setFocus();
    this->ui->lblErrorMessage->setStyleSheet("QLabel {color : red; }");
    this->ui->lblErrorMessage->setVisible(false);
}

void LoginWindow::getUserInformation(){
    QString stringUrl = USER_ENDPOINT_BASE;
    stringUrl = stringUrl+"users/"+this->userId+".json?auth="+this->authToken;
    QUrl getUserUrl = QUrl(stringUrl);
    QNetworkAccessManager * networkManager = new QNetworkAccessManager(this);

    QNetworkRequest request(getUserUrl);

    connect(networkManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(onFinishGetUser(QNetworkReply*)));
    connect(networkManager,SIGNAL(finished(QNetworkReply*)),networkManager,SLOT(deleteLater()));

    networkManager->get(request);
}
void LoginWindow::onFinishGetUser(QNetworkReply *reply){
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    int status = statusCode.toInt();
    QByteArray bytesResponse = reply->readAll();
    if ( status == 200 )
    {
        qDebug()<<"Get user ok\n";
        QJsonObject jsonResponse= QJsonDocument::fromJson(bytesResponse).object();
        //this->connectToUserChanges();
        if(jsonResponse.value("isStadium").toBool()){
            qDebug()<<"Is stadium";
            this->goToMainScreen(jsonResponse);
        }
    }
}

void LoginWindow::goToMainScreen(QJsonObject userResponse){
    mainScreen = new MainWindow();
    mainScreen->setAuth(this->authToken,this->userId,userResponse);
    mainScreen->setUpWindow();
    this->mainScreen->showScreen();
    this->hide();
}
