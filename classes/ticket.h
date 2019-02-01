#ifndef TICKET_H
#define TICKET_H
#include <QJsonValue>
#include <QDateTime>
#include <QObject>

class Ticket
{
public:
    Ticket();
    Ticket(QJsonValue plainTicket, QString key);
    QString getTicketId();
    QString getOwner();
    QString getStatus();
    QString getPayStartAt(QString dateFormat);
    QString getMatchStartAt();
    bool isPay();
    bool isPending();
    bool isSelected();
    void toogleSelectedState();
    void editFromString(QString change,QString data);
private:
    QString ticketId;
    QString owner;
    QString status;
    QDateTime payStartAt;
    QString matchStartAt;
    bool selected;
};
#endif // TICKET_H
