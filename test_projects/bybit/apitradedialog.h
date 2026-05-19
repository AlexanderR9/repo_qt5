#ifndef APITRADEDIALOG_H
#define APITRADEDIALOG_H

#include "lsimpledialog.h"


//Orepation Type
enum TradeOrepationType {totBuyLimit = 771, totSellLimit, totTakeProfit, totStopLoss,
                         totCancel, totModify, totNone = 0};

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
    QString custom_id;

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
    virtual void fillLots();
    void updateKindWidget();
    void updateToStrikeWidget();

    QString genCustomId() const;

protected slots:
    virtual void slotRecalcAwardByDeviation();
    virtual void slotApply();

private:
    quint8 precision() const {return 2;}

};


//APILinearTradeDialog
class APILinearTradeDialog : public APITradeDialog
{
    Q_OBJECT
public:
    APILinearTradeDialog(TradeOperationData&, QWidget*);
    virtual ~APILinearTradeDialog() {}

protected:
    virtual void reinitWidgeys();
    void fillLots();

protected slots:
    virtual void slotRecalcAwardByDeviation();
    virtual void slotApply();

};


//APILinearCancelDialog
class APILinearCancelDialog : public APILinearTradeDialog
{
    Q_OBJECT
public:
    APILinearCancelDialog(TradeOperationData&, QWidget*);
    virtual ~APILinearCancelDialog() {}

protected:
    virtual void reinitWidgeys();

protected slots:
    virtual void slotRecalcAwardByDeviation();
    virtual void slotApply();

};



//APILinearModifyDialog
class APILinearModifyDialog : public APILinearCancelDialog
{
    Q_OBJECT
public:
    APILinearModifyDialog(TradeOperationData&, QWidget*);
    virtual ~APILinearModifyDialog() {}

protected:
    virtual void reinitWidgeys();

protected slots:
    virtual void slotApply();

};






#endif // APITRADEDIALOG_H


