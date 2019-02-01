#include "screens/mainwindow.h"
#include "ui_mainwindow.h"
#include "../models/ticketsmodel.h"
#include "../classes/ticket.h"

#include<QJsonDocument>
#include<QJsonObject>
#include<QNetworkAccessManager>
#include<QUrl>
#include<QDesktopWidget>
#include<QMessageBox>

#define STADIUM_ENDPOINT_BASE "https://tu-partido-video.firebaseio.com/"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}


void MainWindow::setAuth(QString authToken,QString userId, QJsonObject userData)
{
    this->authToken=authToken;
    this->userId=userId;
    this->userData=userData;
}
void MainWindow::showScreen(){
    this->show();
    this->centerScreen();
    this->getStadiumInformation();
}

void MainWindow::centerScreen(){
    QDesktopWidget wid;

    int screenWidth = wid.screen()->width();
    int screenHeight = wid.screen()->height();

    this->setGeometry((screenWidth/2)-(this->width()/2),(screenHeight/2)-(this->height()/2),this->width(),this->height());
}

void MainWindow::setUpWindow(){
    this->ui->btnPayTickets->setEnabled(false);
    this->ui->tableViewTickets->setModel(&this->ticketsModel);

    this->ticketsModel.setAuth(this->authToken);
    this->ticketsModel.setStadiumId(this->userData.value("stadiumId").toString());

    this->ui->tableViewTickets->horizontalHeader()->setStretchLastSection(true);
    this->ui->tableViewTickets->setSelectionMode(QAbstractItemView::NoSelection);
    //connect signals and slot
    connect(this,SIGNAL(currentDateChanged()),this,SLOT(onCurrentDateChange()));

    connect(this, SIGNAL(toogleSelectedStatus(int)), this->ui->tableViewTickets->model(), SLOT(onToogleSelectedStatus(int)));
    connect(this, SIGNAL(confirmPay(QString, QString, QString)), this->ui->tableViewTickets->model(), SLOT(paySelectedItems(QString, QString, QString)));
    connect(this->ui->tableViewTickets, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onTableClicked(const QModelIndex &)));

    connect(this->ui->tableViewTickets->model(), SIGNAL(onItemsSelectedChange(bool)), this, SLOT(tooglePayButtom(bool)));
    connect(this->ui->tableViewTickets->model(), SIGNAL(startLoadingTickets()), this, SLOT(onLockDateControls()));
    connect(this->ui->tableViewTickets->model(), SIGNAL(finishLoadingTickets()), this, SLOT(onUnLockDateControls()));
    connect(this->ui->tableViewTickets->model(), SIGNAL(finishLoadingTickets()), this, SLOT(onDataChanged()));

    connect(this, SIGNAL(startListenTickets()), this->ui->tableViewTickets->model(), SLOT(startListen()));
    connect(this, SIGNAL(stopListenTickets()), this->ui->tableViewTickets->model(), SLOT(stopListen()));
    connect(this, SIGNAL(dateModelChange(QString)), this->ui->tableViewTickets->model(), SLOT(dateChanged(QString)));


    date = QDate::currentDate();
    emit currentDateChanged();

}
void MainWindow::tooglePayButtom(bool status){
    this->ui->btnPayTickets->setEnabled(status);
}
void MainWindow::onTableClicked(const QModelIndex &index){
    emit toogleSelectedStatus(index.row());
}
void MainWindow::onCurrentDateChange(){
    //this->ticketsModel.setDate(date.toString("dd-MM-yyyy"));
    ui->lblSelectedDate->setText(date.toString("dd-MM-yyyy"));
    this->ui->statusBar->showMessage("Obteniendo el listado de pagos...");
    emit dateModelChange(date.toString("dd-MM-yyyy"));
}

void MainWindow::onDataChanged(){
    this->ui->statusBar->showMessage("Listado recuperado.",3000);
}
void MainWindow::onLockDateControls(){
    ui->btnAddDay->setEnabled(false);
    ui->btnSubDay->setEnabled(false);
    ui->btnPayTickets->setEnabled(false);
    ui->tableViewTickets->setEnabled(false);
}
void MainWindow::onUnLockDateControls(){
    ui->btnAddDay->setEnabled(true);
    ui->btnSubDay->setEnabled(true);
    ui->btnPayTickets->setEnabled(true);
    ui->tableViewTickets->setEnabled(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::getStadiumInformation(){
    QString stringUrl = STADIUM_ENDPOINT_BASE;

    stringUrl = stringUrl+"stadiums/"+this->userData.value("stadiumId").toString()+".json?auth="+this->authToken;
    QUrl getUserUrl = QUrl(stringUrl);
    QNetworkAccessManager * networkManager = new QNetworkAccessManager(this);

    QNetworkRequest request(getUserUrl);

    connect(networkManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(onFinishGetStadium(QNetworkReply*)));
    connect(networkManager,SIGNAL(finished(QNetworkReply*)),networkManager,SLOT(deleteLater()));

    networkManager->get(request);
}
void MainWindow::onFinishGetStadium(QNetworkReply *reply){
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    int status = statusCode.toInt();
    QByteArray bytesResponse = reply->readAll();
    if ( status == 200 )
    {
        qDebug()<<"Get stadium ok\n";
        QJsonObject jsonResponse= QJsonDocument::fromJson(bytesResponse).object();
        foreach(QString key,jsonResponse.keys()){
            qDebug()<<"key:"<<key<<"-value:"<<jsonResponse.value(key).toString();
        }
        ui->lblDirection->setText(jsonResponse.value("direction").toString()+","+jsonResponse.value("location").toString());
        ui->lblStadiumName->setText(jsonResponse.value("name").toString());
    }
}

void MainWindow::on_btnAddDay_clicked()
{
    date = date.addDays(1);
    emit currentDateChanged();
}

void MainWindow::on_btnSubDay_clicked()
{
    date = date.addDays(-1);
    emit currentDateChanged();
}

void MainWindow::on_btnPayTickets_clicked()
{
    int numTickets = this->ticketsModel.getTotalToPay();

    QString message = QString("Confirmar el pago de ");

    if(numTickets==1){
        message = message+QString::number(numTickets)+QString(" ticket");
    }else{
        message = message+QString::number(numTickets)+QString(" tickets");
    }

    QMessageBox msgBox(QMessageBox::Question,"Confirmar pago",message, QMessageBox::Yes | QMessageBox::No);

    msgBox.setButtonText(QMessageBox::Yes,"Yes");
    msgBox.setButtonText(QMessageBox::No, "No");

    if (msgBox.exec() == QMessageBox::Yes) {
        emit confirmPay(this->userData.value("stadiumId").toString(),this->authToken,date.toString("dd-MM-yyyy"));
    }
}
