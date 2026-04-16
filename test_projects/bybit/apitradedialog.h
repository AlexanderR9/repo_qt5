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
    float lot_size;
    float asset_price; // cur price of ETH
    float award;    // market award of OPTION
    float strike;
    QString ticker;
    QString type; // CALL / PUT
    quint32 expirate;
    //int maxLot_ps;

    void reset() {lot_size = award = 0; asset_price = strike = -1;  expirate = 0;}

    bool invalidType() const {return (order_type < totBuyLimit || order_type > totStopLoss);}
    bool invalid() const {return (invalidType() || lot_size < 0.1 || asset_price<=0 || strike <= 0 || ticker.trimmed().isEmpty());}
    QString toStr() const {return QString("TradeOperationData: order_type=%1, lots=%2, price=%3").arg(order_type).arg(lot_size).arg(award);}

    bool isCall() const {return (type.trimmed().toLower() == "call");}
    bool downKind() const {return (order_type == totBuyLimit || order_type == totStopLoss);}
    bool upKind() const {return (order_type == totSellLimit || order_type == totTakeProfit);}
    bool redKind() const {return (order_type == totSellLimit || order_type == totStopLoss);}
    bool isBuy() const {return (order_type == totBuyLimit);}
    bool isSell() const {return (order_type == totSellLimit);}


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
    //void fillDeviations();
    void updateKindWidget();
    void updateToStrikeWidget();



protected slots:
    void slotRecalcAwardByDeviation();
    void slotApply();


private:
    quint8 precision() const;
    //void trimPrice();


};



#endif // APITRADEDIALOG_H


