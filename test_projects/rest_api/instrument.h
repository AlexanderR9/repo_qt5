#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "lstring.h"

#include <QString>
#include <QDate>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>


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

    static quint8 filedsCount() {return 14;}

    void reset();
    QString toStr() const;
    void fromJson(const QJsonValue&);
    QStringList toTableRowData() const;
    void parseFields(const QStringList&);

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




#endif

