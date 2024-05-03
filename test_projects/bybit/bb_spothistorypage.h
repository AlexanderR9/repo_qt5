#ifndef BB_SPOTHISTORYPAGE_H
#define BB_SPOTHISTORYPAGE_H

#include "bb_historypage.h"



//BB_SpotHistoryPage
class BB_SpotHistoryPage : public BB_HistoryPage
{
    Q_OBJECT
public:
    BB_SpotHistoryPage(QWidget*);
    virtual ~BB_SpotHistoryPage() {}


    QString caption() const {return QString("History (SPOT)");}

protected:
    QList<BB_HistorySpot>   m_spotOrders;
    QDate   m_startDate;
    int     m_polledDays;

    virtual void loadTablesByContainers(); //by start program once
    virtual void goExchange(const QJsonObject &jresult_obj);
    virtual void loadContainers(); //load m_posList, m_orderList from files by start program

    void sendOrdersReq();
    void sendNextOrdersReq();
    void parseOrders(const QJsonObject&);
    void insertSpotEvent(const BB_HistorySpot&);
    bool hasSpotEvent(const QString&) const;

    void reinitWidgets();
    void fillTable(const QJsonArray&);
    int needPollDays() const;
    void getTSNextInterval(qint64&, qint64&);
    void updateLastRowColors(const BB_HistorySpot&);


};




#endif // BB_SPOTHISTORYPAGE_H


