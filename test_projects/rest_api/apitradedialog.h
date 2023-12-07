#ifndef APITRADEDIALOG_H
#define APITRADEDIALOG_H

#include "lsimpledialog.h"


//Orepation Type
enum TradeOrepationType {totBuyLimit = 771, totSellLimit, totNone = 0};

//TradeOperationData
struct TradeOperationData
{
    TradeOperationData() :order_type(totNone) {reset();}
    TradeOperationData(int t) :order_type(t) {reset();}

    int order_type;
    quint16 lots;
    float price;
    float coupon;
    QString company;
    QString paper_type;
    QString finish_date;

    void reset() {lots = 1; price = -1; coupon = 0;}
    bool invalidType() const {return (order_type < totBuyLimit || order_type > totSellLimit);}
    bool invalid() const {return (invalidType() || lots==0 || price<=0 || company.trimmed().isEmpty());}
    QString toStr() const {return QString("TradeOperationData: order_type=%1, lots=%2, price=%3").arg(order_type).arg(lots).arg(price);}
    bool isBond() const {return (paper_type == "bond");}

};

//APITradeDialog
class APITradeDialog : public LSimpleDialog
{
    Q_OBJECT
public:
    APITradeDialog(TradeOperationData&, QWidget*);
    virtual ~APITradeDialog() {}

protected:
    TradeOperationData&     m_data;

    void init();
    void updateTitle();
    void updateWidgetsSizePolicy();

protected slots:
    void slotRecalcResult();


};



#endif // APITRADEDIALOG_H


