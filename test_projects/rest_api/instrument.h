#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "lstring.h"

#include <QString>
#include <QDate>
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

    quint16 number; //порядковый номер
    QString name;
    QString uid;
    QString figi;
    QString country;
    QString currency;
    bool api_trade;

    static QString userDateMask() {return QString("dd.MM.yyyy");}
    static QDate dateFromGoogleDT(const QJsonValue&);
    static float floatFromJVBlock(const QJsonValue&);

    virtual void reset() {number = 0; name = "?"; country = currency = uid = figi = QString(); api_trade = false;}
    virtual bool invalid() const {return (name.isEmpty() || uid.isEmpty());}
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

    QString ticker;
    QString sector;

    static quint8 filedsCount() {return 8;}

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



#endif

