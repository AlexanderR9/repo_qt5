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

    void reinitWidgets();
    void fillTable(const QJsonArray&);


};




#endif // BB_SPOTHISTORYPAGE_H


