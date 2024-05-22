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

    virtual void loadTablesByContainers(); //by start program once
    virtual void goExchange(const QJsonObject &jresult_obj);
    virtual void loadContainers(); //load m_posList, m_orderList from files by start program
    virtual quint8 daysSeparator() const {return 6;}

    void sendOrdersReq();
    void sendNextOrdersReq();
    void parseOrders(const QJsonObject&);
    void insertSpotEvent(const BB_HistorySpot&);
    bool hasSpotEvent(const QString&) const;

    void reinitWidgets();
    void fillTable(const QJsonArray&);
    void updateLastRowColors(const BB_HistorySpot&);


};




#endif // BB_SPOTHISTORYPAGE_H


