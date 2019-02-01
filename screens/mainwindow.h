#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "../models/ticketsmodel.h"
#include "../classes/ticket.h"

#include <QMainWindow>
#include <QJsonObject>
#include <QDate>
#include <QNetworkReply>
#include <QMap>
#include <QList>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showScreen();
    void setUpWindow();
    void setAuth(QString authToken,QString userId, QJsonObject userData);

private slots:
     void onFinishGetStadium(QNetworkReply *reply);
     void on_btnAddDay_clicked();
     void on_btnSubDay_clicked();
     void onCurrentDateChange();
     void centerScreen();
     void onTableClicked(const QModelIndex &index);
     void tooglePayButtom(bool status);
     void on_btnPayTickets_clicked();

public slots:
     void onLockDateControls();
     void onUnLockDateControls(bool isEmpty);
     void onInsertTickets();

signals:
     void startListenTickets();
     void stopListenTickets();
     void dateModelChange(QString date);
     void currentDateChanged();
     void confirmPay(QString stadiumId,QString auth,QString date);
     void lockDateControls();
     void unLockDateControls();
     void showStatusBarMessage(QString message);
     void toogleSelectedStatus(int row);


private:
    Ui::MainWindow *ui;
    QString authToken;
    QString userId;
    QJsonObject userData;
    void getStadiumInformation();
    void onDataChanged(bool isEmpty);
    QDate date;
    TicketsModel ticketsModel;

};

#endif // MAINWINDOW_H
