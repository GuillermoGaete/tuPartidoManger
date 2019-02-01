#ifndef TICKETSMODEL_H
#define TICKETSMODEL_H

#include <QAbstractTableModel>
#include "../classes/ticket.h"
#include "../services/networkmanager.h"

class TicketsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    TicketsModel(QObject *parent);
    TicketsModel();
    int getTotalToPay();

    int rowCount(const QModelIndex &parent = QModelIndex()) const override ;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;    
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool addTicket(Ticket newTicket);
    void setDate(QString date);
    void setAuth(QString auth);
    void setStadiumId(QString stadiumId);

    bool updateTicket(Ticket updatedTicket);

    bool removeAllTickets();

signals:
    void onItemsSelectedChange(bool status);
    void finishLoadingTickets(bool isEmpty);
    void startLoadingTickets();
    void insertTickets();

private slots:
    void onTicketsDataChanged(QByteArray array,int statusCode);
    void payTicket(Ticket ticket,QString stadiumId, QString auth, QString date);
    void onFinishPayTicket(QNetworkReply *reply);

public slots:
    void onToogleSelectedStatus(int row);
    void dateChanged(QString date);
    void startListen();
    void stopListen();
    void paySelectedItems(QString stadiumId, QString auth, QString date);

private:
    QVector<Ticket> tickets;
    bool isFirtsChange;
    QString date,stadiumId,auth;
    void processChangeOnTickets(QString path,QJsonValue data, QString event);
    bool editTicket(QString change,QString ticketId,QString data);
    bool deleteTicket(QString ticketId);
    bool fisrtRequest;

};
#endif // TICKETSMODEL_H
