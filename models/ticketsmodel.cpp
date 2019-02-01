#include "ticketsmodel.h"
#include "../classes/ticket.h"
#include "../services/networkmanager.h"
#include <QAbstractTableModel>
#include <QFont>
#include <QBrush>
#include <QVector>
#include <QTime>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QBuffer>

TicketsModel::TicketsModel(QObject *parent)
    :QAbstractTableModel(parent)
{
}

TicketsModel::TicketsModel()
    :QAbstractTableModel()
{
}

bool TicketsModel::addTicket(Ticket newTicket){

    int newRow = this->tickets.count();
    beginInsertRows(QModelIndex(), newRow, newRow);
    this->tickets.append(newTicket);
    endInsertRows();

    return true;
}

void TicketsModel::startListen(){
    qDebug()<<"Start listen\n";
    Network::Manager *manager = Network::Manager::getInstance();
    QString stringUrl = "https://tu-partido-video.firebaseio.com/";
    stringUrl = stringUrl+"stadiums-tickets/"+this->stadiumId+"/"+this->date+".json?auth="+this->auth;
    qDebug()<<stringUrl<<"\n\n";
    QUrl getTicketsUrl = QUrl(stringUrl);
    manager->getResource(QUrl(getTicketsUrl));
    connect(manager,SIGNAL(dataRecived(QByteArray,int)),this,SLOT(onTicketsDataChanged(QByteArray,int)));
    if(tickets.empty()){
        this->fisrtRequest=true;
        emit startLoadingTickets();
    }
}

void TicketsModel::stopListen(){
    Network::Manager *manager = Network::Manager::getInstance();
    manager->clearConnectionCache();
    disconnect(manager,SIGNAL(dataRecived(QByteArray,int)),this,SLOT(onTicketsDataChanged(QByteArray,int)));
    qDebug()<<"Stop listen\n";
}


void TicketsModel::paySelectedItems(QString stadiumId, QString auth, QString date){
    foreach (Ticket currenTicket, this->tickets) {
        if(currenTicket.isSelected()){
            this->payTicket(currenTicket,stadiumId,auth,date);
        }
    }
}

void TicketsModel::dateChanged(QString date){
    this->date=date;
    this->fisrtRequest=true;
    this->stopListen();
    this->removeAllTickets();
    this->startListen();
}

int TicketsModel::getTotalToPay(){
    int count = 0;
    foreach (Ticket currenTicket, this->tickets) {
        if(currenTicket.isSelected()){
            count++;
        }
    }
    return count;
}

void TicketsModel::payTicket(Ticket ticket,QString stadiumId, QString auth, QString date){
    QString stringUrl = "https://tu-partido-video.firebaseio.com/";
    stringUrl = stringUrl+"stadiums-tickets/"+stadiumId+"/"+date+"/"+ticket.getTicketId()+".json?auth="+auth;

    QString jsonString=QString(QString("{").append("\"").append("status").append("\"").append(":").append("\"").append("pay").append("\"").append(QString("}")));
    QNetworkRequest request(stringUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/json");

    QBuffer *buffer=new QBuffer();
    buffer->open((QBuffer::ReadWrite));
    buffer->write(jsonString.toUtf8());
    buffer->seek(0);
    QNetworkAccessManager * networkManager = new QNetworkAccessManager(this);

    connect(networkManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(onFinishPayTicket(QNetworkReply*)));
    connect(networkManager,SIGNAL(finished(QNetworkReply*)),networkManager,SLOT(deleteLater()));

    networkManager->sendCustomRequest(request,"PATCH",buffer);

    this->editTicket("status",ticket.getTicketId(),"pay");
}
void TicketsModel::onFinishPayTicket(QNetworkReply *reply){
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if(statusCode==200){
        qDebug()<<"Finish pay ticket";
    }else{
        qDebug()<<"Error on pay ticket";
    }
}
void TicketsModel::onToogleSelectedStatus(int row){
    Ticket ticket = this->tickets.at(row);
    ticket.toogleSelectedState();
    this->tickets.replace(row,ticket);
    QModelIndex top = createIndex(row,0);
    QModelIndex bottom = createIndex(row,6);
    emit dataChanged(top,bottom);
    bool thereAreSelecteds=false;
    foreach (Ticket currenTicket, this->tickets) {
        if(currenTicket.isSelected()){
            thereAreSelecteds=true;
        }
    }
    emit onItemsSelectedChange(thereAreSelecteds);
}

void TicketsModel::onTicketsDataChanged(QByteArray array,int statusCode){

    QByteArray originalArray = array;
    int fisrtIndexEndLine = array.indexOf("\n");
    QByteArray event = array.left(fisrtIndexEndLine).remove(0,7);

    if(event=="keep-alive"){
        return;
    }
    if(event=="patch"){
        return;
    }

    int lastIndexCloseJson = array.lastIndexOf("}");
    int fisrtIndexCloseJson = array.indexOf("{");

    array.truncate(lastIndexCloseJson+1);
    array.remove(0, fisrtIndexCloseJson-1);
    QJsonParseError error;

    QJsonDocument responseDocument= QJsonDocument::fromJson(array.data(),&error);
    if(error.error==QJsonParseError::NoError){

        QString path= responseDocument.object().value("path").toString();
        QJsonValue data= responseDocument.object().value("data");
        this->processChangeOnTickets(path,data,QString(event));
    }else{
        qDebug()<<"Error on parse:"<<error.errorString();
        qDebug()<<"statusCode"<<statusCode<<"-array:"<<originalArray<<"\n";
    }
}

void TicketsModel::processChangeOnTickets(QString path,QJsonValue data, QString event){
    if(this->fisrtRequest){
        this->fisrtRequest=false;
        emit finishLoadingTickets();
    }
    if(event=="put"){
        if(path=="/"){//se recibieron varios tickets
            QJsonObject dataJsonObject = data.toObject();
            foreach (QString key, dataJsonObject.keys()) {
                QString ticketId = key;
                QJsonValue jsonTicket = dataJsonObject.value(key);
                Ticket newTicket(jsonTicket, ticketId);
                this->addTicket(newTicket);
            }
        }else{
            path.remove(0,1);
            QStringList partsPath = path.split("/");
            if(partsPath.count()==1){
                QString ticketId = partsPath.at(0);
                QJsonValue jsonTicket = data;
                if(jsonTicket.isNull()){
                    this->deleteTicket(ticketId);
                }else{
                    Ticket newTicket(jsonTicket, ticketId);
                    this->addTicket(newTicket);
                }
            }else if(partsPath.count()==2){
                QString ticketId = partsPath.at(0);
                QString change = partsPath.at(1);
                this->editTicket(change,ticketId,data.toString());
            }else{
                qDebug()<<"No se reconoce el evento:"<<event<<", con el path: "<<path<<"\n";
            }
        }
    }
}

bool TicketsModel::editTicket(QString change,QString ticketId,QString data){
        int index=0;
        foreach(Ticket currentTicket, this->tickets){
            if(currentTicket.getTicketId()==ticketId){
                currentTicket.editFromString(change,data);
                this->tickets.replace(index,currentTicket);
                QModelIndex top = createIndex(index,0);
                QModelIndex bottom = createIndex(index,6);
                emit dataChanged(top,bottom);
            }
            index++;
        }
        return true;
}
bool TicketsModel::removeAllTickets(){
    if(tickets.empty()){
        return false;
    }
    beginResetModel();
    this->tickets.clear();
    endResetModel();
    return true;
}
bool TicketsModel::deleteTicket(QString ticketId){
    int index=0;
    foreach(Ticket currentTicket, this->tickets){
        if(currentTicket.getTicketId()==ticketId){
            beginRemoveRows(QModelIndex(),index,index);
            this->tickets.removeAt(index);
            endRemoveRows();
        }
        index++;
    }
    return true;
}

int TicketsModel::rowCount(const QModelIndex & /*parent*/) const
{
   int rowCount = this->tickets.count();
   return rowCount;
}

int TicketsModel::columnCount(const QModelIndex & /*parent*/) const
{
    return 6;
}
QVariant TicketsModel::headerData(int section, Qt::Orientation orientation, int role) const
{

    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal) {
            switch (section)
            {
            case 0:
                return QString("");
            case 1:
                return QString("Concepto");
            case 2:
                return QString("Concepto");
            case 3:
                return QString("Hora de partido");
            case 4:
                return QString("Hora generado");
            case 5:
                return QString("Estado");
            }
        }
    }
    return QVariant();
}

QVariant TicketsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();
    // generate a log message when this method gets called
    Ticket currentTicket = this->tickets[row];

    switch(role){
    case Qt::DisplayRole:
            if(col==1){
            return currentTicket.getOwner();
        }
        if(col==2){
            return QString("Pago de video");
        }
        if(col==3){
             return currentTicket.getMatchStartAt();
        }
        if(col==4){
             return currentTicket.getPayStartAt("hh:mm");
        }
        if(col==5){
             return currentTicket.getStatus();
        }
        return QVariant();
        break;
    case Qt::FontRole:
        if (col == 5) //change font only for col status pay
        {
            QFont boldFont;
            boldFont.setBold(true);
            return boldFont;
        }
        break;
    case Qt::BackgroundColorRole:
        if (col == 5) //change font only for col status pay
        {
            if(currentTicket.isPending()){
                if(currentTicket.isSelected()){
                    QColor red = QColor::fromRgb(174, 214, 241);
                    return QVariant(red);
                }
                QColor red = QColor::fromRgb(255, 204, 204);
                return QVariant(red);
            }
        }else{
            if(currentTicket.isSelected()){
                QColor red = QColor::fromRgb(174, 214, 241);
                return QVariant(red);
            }else{
                return QVariant();
            }
        }
        break;
    case Qt::CheckStateRole:
            if (col == 0) //add a checkbox to cell(1,0)
            {
                if(currentTicket.isSelected()){
                    return Qt::Checked;
                }else{
                    return Qt::Unchecked;
                }
            }
        break;
    }
    return QVariant();
}

void TicketsModel::setDate(QString date){
    this->date=date;
}

void TicketsModel::setAuth(QString auth){
    this->auth=auth;
}

void TicketsModel::setStadiumId(QString stadiumId){
    this->stadiumId=stadiumId;
}
