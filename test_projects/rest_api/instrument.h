#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "lstring.h"

#include <QString>
#include <QDate>
#include <QDebug>

class QDomNode;
class QDomDocument;
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

//struct calendar bond coupon pays info
struct BCoupon
{
    BCoupon() {reset();}
    BCoupon(const QString &id) {reset(); figi=id;}

    quint16 number;
    QString figi;
    QDate pay_date;
    float pay_size;
    quint16 period;

    void reset() {figi.clear(); number = 9999; pay_date = QDate(); pay_size = -1; period = 0;}
    bool invalid() const {return (!pay_date.isValid() || number > 1000 || pay_size <= 0 || period == 0 || figi.isEmpty());}
    void syncData(QDomNode&, QDomDocument&);
    void toNode(QDomNode&, QDomDocument&);
    void fromNode(QDomNode&);
    QString toString() const {return QString("BCoupon: %1 - N=%2  date[%3] size=%4").arg(figi).arg(number).arg(pay_date.toString(InstrumentBase::userDateMask())).arg(pay_size);}
    float daySize() const;
    int daysTo() const;
    void fromJson(const QJsonObject&);


};


#endif

