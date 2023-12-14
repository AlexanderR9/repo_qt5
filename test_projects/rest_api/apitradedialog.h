#ifndef APITRADEDIALOG_H
#define APITRADEDIALOG_H

#include "lsimpledialog.h"


//Orepation Type
enum TradeOrepationType {totBuyLimit = 771, totSellLimit, totTakeProfit, totStopLoss, totNone = 0};

//TradeOperationData
struct TradeOperationData
{
    TradeOperationData() :order_type(totNone) {reset();}
    TradeOperationData(int t) :order_type(t) {reset();}

    int order_type;
    quint16 lots;
    quint16 in_lot; //count ps in one lot
    float price;
    float coupon;
    QString company;
    QString paper_type;
    QString finish_date;
    int maxLot_ps;

    void reset() {lots = in_lot = 1; price = -1; coupon = 0; maxLot_ps = -1;}
    bool invalidType() const {return (order_type < totBuyLimit || order_type > totStopLoss);}
    bool invalid() const {return (invalidType() || lots==0 || price<=0 || company.trimmed().isEmpty());}
    QString toStr() const {return QString("TradeOperationData: order_type=%1, lots=%2, price=%3").arg(order_type).arg(lots).arg(price);}
    bool isBond() const {return (paper_type == "bond");}
    bool downKind() const {return (order_type == totBuyLimit || order_type == totStopLoss);}
    bool upKind() const {return (order_type == totSellLimit || order_type == totTakeProfit);}
    bool redKind() const {return (order_type == totSellLimit || order_type == totStopLoss);}

};

//APITradeDialog
class APITradeDialog : public LSimpleDialog
{
    Q_OBJECT
public:
    APITradeDialog(TradeOperationData&, QWidget*);
    virtual ~APITradeDialog() {}

    static QString captionByOrderType(int);
    static QString iconByOrderType(int);

protected:
    TradeOperationData&     m_data;


    void init();
    void updateTitle();
    void updateWidgetsSizePolicy();
    void fillLots();
    void fillDeviations();
    void updateKindWidget();



protected slots:
    void slotRecalcResult();

private:
    quint8 precision() const;
    void trimPrice();


};



#endif // APITRADEDIALOG_H


