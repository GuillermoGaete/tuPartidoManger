#include "ticket.h"
#include <QJsonObject>
#include <QDebug>
#include <QDateTime>
#include <QObject>
#include <QUrl>
#include <QJsonDocument>

Ticket::Ticket()
{

}
Ticket::Ticket(QJsonValue plainTicket,QString key)
{
    QJsonObject ticketJsonObject = plainTicket.toObject();
    QString payStartAt = ticketJsonObject.value("payStartAt").toString();
    this->payStartAt = QDateTime::fromString(payStartAt,"dd-MM-yyyy:hh:mm:ss");
    this->matchStartAt= ticketJsonObject.value("matchStartAt").toString();
    this->owner= ticketJsonObject.value("owner").toString();
    this->status = ticketJsonObject.value("status").toString();
    this->ticketId = key;
    this->selected=false;
}

QString Ticket::getOwner(){
    return this->owner;
}
void Ticket::toogleSelectedState(){
    this->selected=!this->selected;
}
bool Ticket::isPending(){
    if(this->status=="pending"){
        return true;
    }else{
        return false;
    }
}
bool Ticket::isSelected(){
    return this->selected;
}
void Ticket::editFromString(QString change, QString data){
    if(change=="owner"){
        this->owner=data;
    }
    if(change=="status"){
        this->status=data;
    }
}
bool Ticket::isPay(){
    if(this->status=="pay"){
        return true;
    }else{
        return false;
    }
}
QString Ticket::getStatus(){
    QString parsedStatus="sin informacion";
    if(this->status=="pending"){
        parsedStatus="Pendiente";
    }else{
        parsedStatus="Pago";
    }
    return parsedStatus;
}
QString Ticket::getPayStartAt(QString dateFormat){
    return this->payStartAt.toString(dateFormat);
}
QString Ticket::getMatchStartAt(){
    return this->matchStartAt;
}

QString Ticket::getTicketId(){
    return this->ticketId;
}
