#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "lstring.h"

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QDebug>

class QDomNode;
class QDomDocument;
class QDomElement;
class QJsonArray;
class QJsonObject;
class QJsonValue;



//InstrumentBase
struct InstrumentBase
{
    InstrumentBase() {reset();}
    virtual ~InstrumentBase() {}

    quint16 number; //порядковый номер
    QString name;
    QString uid;
    QString figi;
    QString country;
    QString currency;
    bool api_trade;

    static QString userDateMask() {return QString("dd.MM.yyyy");}
    static QString userTimeMask() {return QString("hh:mm:ss");}
    static QDate dateFromGoogleDT(const QJsonValue&);
    static QDateTime dateTimeFromGoogleDT(const QJsonValue&);
    static float floatFromJVBlock(const QJsonValue&);
    static void floatToJVBlock(float, QJsonObject&);


    virtual void reset() {number = 0; name = "?"; country = currency = uid = figi = QString(); api_trade = false;}
    virtual bool invalid() const {return (name.isEmpty() || uid.isEmpty() || figi.isEmpty());}
    virtual void fromFileLine(const QString&);

    virtual QString toStr() const = 0;
    virtual void fromJson(const QJsonValue&) = 0;
    virtual QStringList toTableRowData() const = 0;
    virtual void parseFields(const QStringList&) = 0;

};

// bond params description
struct BondDesc : public InstrumentBase
{
    BondDesc() {reset();}
    BondDesc(const QJsonValue &jv) {fromJson(jv);}
    virtual ~BondDesc() {}

    QString isin; //kks
    QString risk;
    int coupons_year; // coupon_quantity_per_year
    float cur_coupon_size; //aci_value
    QDate start_date; //placement_date
    QDate finish_date; //maturity_date
    bool amortization;
    bool floating_coupon;
    float nominal;

    static quint8 filedsCount() {return 15;}

    void reset();
    QString toStr() const;
    void fromJson(const QJsonValue&);
    QStringList toTableRowData() const;
    void parseFields(const QStringList&);
    int daysToFinish() const;

private:
    QString getCurrency(const QJsonObject&) const;
    QString getRisk(const QJsonObject&) const;

};

// stock params description
struct StockDesc : public InstrumentBase
{
    StockDesc() {reset();}
    StockDesc(const QJsonValue &jv) {fromJson(jv);}
    virtual ~StockDesc() {}

    QString ticker;
    QString sector;
    quint32 in_lot; //count papers in one lot

    static quint8 filedsCount() {return 9;}

    void reset();
    QString toStr() const;
    void fromJson(const QJsonValue&);
    QStringList toTableRowData() const;
    void parseFields(const QStringList&);

};



/////////////////////// coupon structures /////////////////////////////////////////
struct CouponRecordAbstract
{
    CouponRecordAbstract() {reset();}
    virtual ~CouponRecordAbstract() {}

    QString figi;
    QDate date;
    float size;

    virtual void reset() {figi.clear(); date = QDate(); size = -1;}
    virtual bool invalid() const {return (!date.isValid() || size <= 0 || figi.isEmpty());}
    virtual QString toString() const = 0;
    virtual void syncData(QDomNode&, QDomDocument&);
    virtual void toNode(QDomNode&, QDomDocument&);
    virtual void fromNode(QDomNode&) = 0;
    virtual int daysTo() const;
    virtual void fromJson(const QJsonObject&) = 0;

protected:
    virtual void setAttrs(QDomElement&);

};

//struct calendar bond coupon pays info
struct BCoupon : public CouponRecordAbstract
{
    BCoupon() :CouponRecordAbstract() {reset();}
    BCoupon(const QString &id) :CouponRecordAbstract() {reset(); figi=id;}
    virtual ~BCoupon() {}

    quint16 number;
    quint16 period;

    void reset() {CouponRecordAbstract::reset(); number = 9999; period = 0;}
    bool invalid() const {return (!date.isValid() || number > 1000 || size <= 0 || period == 0 || figi.isEmpty());}
    //void toNode(QDomNode&, QDomDocument&);
    void fromNode(QDomNode&);
    QString toString() const {return QString("BCoupon: %1 - N=%2  date[%3] size=%4").arg(figi).arg(number).arg(date.toString(InstrumentBase::userDateMask())).arg(size);}
    float daySize() const;
    void fromJson(const QJsonObject&);

protected:
    void setAttrs(QDomElement&);

};

//struct calendar stock div info
struct SDiv : public CouponRecordAbstract
{
    SDiv() :CouponRecordAbstract() {reset();}
    SDiv(const QString &id) :CouponRecordAbstract() {reset(); figi=id;}
    virtual ~SDiv() {}

    float yield;
    float last_price;

    void reset() {CouponRecordAbstract::reset(); yield = last_price = -1;}
    bool invalid() const {return (!date.isValid() || size <= 0 || figi.isEmpty());}
    QString toString() const {return QString("SDiv: %1 - date[%2] size=%3(%4%) last_price=%5").arg(figi).arg(date.toString(InstrumentBase::userDateMask())).arg(size).arg(yield).arg(last_price);}
    void fromNode(QDomNode&);
    void fromJson(const QJsonObject&);

protected:
    void setAttrs(QDomElement&);

};

////////////////////////// history of operations ////////////////////////////
struct EventOperation  : public InstrumentBase
{
    enum EventType {etCanceled = 300, etBuy, etSell, etCoupon, etDiv, etCommission, etTax,
                        etInput, etOut, etRepayment, etUnknown = 999};

    EventOperation() {reset();}
    virtual ~EventOperation() {}

    int kind;
    QDateTime date;
    quint32 n_papers;
    float size; //price one paper
    float amount;    
    QString paper_type;

    static quint8 filedsCount() {return 9;}
    static QString dataFile() {return QString("events.txt");}

    bool invalid() const {return (kind < etCanceled || !date.date().isValid());}
    void reset();
    void fromJson(const QJsonValue&);
    QString strKind() const;
    QString toStr() const; //file line
    QStringList toTableRowData() const;
    void parseFields(const QStringList&);
    bool isSame(const EventOperation&) const;
    bool isBond() const {return (paper_type == "bond");}


private:
    void kindByAPIType(QString);
    void parseTradesJson(const QJsonValue&);

};

//OrderData
struct OrderData
{
    OrderData() :is_stop(false) {reset();}
    virtual ~OrderData() {}

    QString type;
    QDateTime time; //дата и время выставления заявки
    QString uid;
    QString currency;
    QString order_id;
    bool is_stop;

    QPair<quint16, quint16> lots; //всего запрошено лотов / уже исполнено лотов
    float price; //запрошенная цена

    virtual bool invalid() const;
    virtual void reset();
    virtual void fromJson(const QJsonValue&);
    virtual QString strLots() const;
    virtual QString toStr() const;

};

//StopOrderData
struct StopOrderData : public OrderData
{
    StopOrderData() :OrderData()  {this->is_stop = true;}
    virtual ~StopOrderData() {}


    virtual void fromJson(const QJsonValue&);
    virtual QString strLots() const;

};

//PlaceOrderData
struct PlaceOrderData
{
    PlaceOrderData() {reset();}

    QString uid;
    QString kind; // buy/sell/cancel
    quint8 is_stop; //0-none, 1-tp, 2-sl
    float price;
    quint16 lots;
    float nominal; //only bond

    void reset();
    bool invalid() const;
    bool isCancel() const {return (kind == "cancel");}
    bool isBuy() const {return (kind == "buy");}
    bool isSell() const {return (kind == "sell");}
    bool isStop() const {return (is_stop == 1 || is_stop == 2);}
    QString toStr() const;

};


#endif

