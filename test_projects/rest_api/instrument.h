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
    quint16 number; //порядковый номер
    QString name;
    QString uid;
    QString country;
    QString currency;

    static QString userDateMask() {return QString("dd.MM.yyyy");}

    virtual void reset() {number = 0; name = "?"; country = currency = uid = QString();}
    virtual bool invalid() const {return (name.isEmpty() || uid.isEmpty());}
};

// bond params description
struct BondDesc : public InstrumentBase
{
    BondDesc() {reset();}
    BondDesc(const QJsonValue &jv) {fromJson(jv);}

    QString isin; //kks
    QString figi;
    QString risk;
    int coupons_year; // coupon_quantity_per_year
    float cur_coupon_size; //aci_value
    QDate start_date; //placement_date
    QDate finish_date; //maturity_date
    bool api_trade;
    bool amortization;
    bool floating_coupon;

    static quint8 filedsCount() {return 14;}

    void reset()
    {
        InstrumentBase::reset();

        //number = 0;
        //name = "?";
        //country = currency = isin = figi = uid = risk = QString();
        isin = figi = risk = QString();
        coupons_year = -1;
        cur_coupon_size = 0;
        start_date = finish_date = QDate();
        api_trade = amortization = floating_coupon;
    }

    QString toStr() const
    {
        //return QString("%1: %2 / %3 / %4    PLACE: %5 (%6)   RISK: %7").arg(name)
          //          .arg(isin).arg(figi).arg(uid).arg(country).arg(currency).arg(risk);
        QString s(name);
        s = QString("%1 / %2 / %3 / %4").arg(s).arg(country).arg(currency).arg(risk);
        s = QString("%1 / %2 / %3 / %4").arg(s).arg(isin).arg(figi).arg(uid);
        s = QString("%1 / %2 / %3").arg(s).arg(coupons_year).arg(QString::number(cur_coupon_size, 'f', 1));
        s = QString("%1 / %2 / %3").arg(s).arg(start_date.toString(userDateMask())).arg(finish_date.toString(userDateMask()));
        s = QString("%1 / %2 / %3 / %4").arg(s).arg(api_trade?"true":"false").arg(amortization?"true":"false").arg(floating_coupon?"true":"false");
        return s;
    }
    static QDate dateFromGoogleDT(const QJsonValue &jv_dt)
    {
        QDate q_date;
        if (jv_dt.isString())
        {
            QString gdt = jv_dt.toString().trimmed();
            int pos = gdt.indexOf("T");
            if (pos > 0)
            {
                gdt = gdt.left(pos);
                q_date = QDate::fromString(gdt.trimmed(), "yyyy-MM-dd");
            }
        }
        return q_date;
    }
    static float floatFromJVBlock(const QJsonValue &jv_block)
    {
        float f = -1;
        if (jv_block.isObject())
        {
            const QJsonObject &f_obj = jv_block.toObject();
            float f_units = f_obj.value("units").toString().toDouble();
            double f_nano = f_obj.value("nano").toDouble();
            //qDebug()<<QString("f_units=%1    f_nano=%2").arg(f_units).arg(f_nano);
            while (f_nano > 1) f_nano /= double(10);
            f = f_units + f_nano;
        }
        return f;
    }
    void fromJson(const QJsonValue &jv)
    {
        reset();
        if (jv.isObject())
        {
            const QJsonObject &j_obj = jv.toObject();
            name = j_obj.value("name").toString();
            country = j_obj.value("countryOfRiskName").toString();
            currency = j_obj.value("currency").toString();
            isin = j_obj.value("isin").toString();
            uid = j_obj.value("uid").toString();
            figi = j_obj.value("figi").toString();

            if (j_obj.value("riskLevel").isDouble()) risk = QString::number(j_obj.value("riskLevel").toDouble());
            else risk = j_obj.value("riskLevel").toString();

            coupons_year = j_obj.value("couponQuantityPerYear").toInt();
            api_trade =  j_obj.value("apiTradeAvailableFlag").toBool();
            amortization =  j_obj.value("amortizationFlag").toBool();
            floating_coupon =  j_obj.value("floatingCouponFlag").toBool();
            start_date = BondDesc::dateFromGoogleDT(j_obj.value("placementDate"));
            finish_date = BondDesc::dateFromGoogleDT(j_obj.value("maturityDate"));
            cur_coupon_size = BondDesc::floatFromJVBlock(j_obj.value("aciValue"));

            if (country.trimmed().isEmpty()) country = "?";
            if (currency.trimmed().isEmpty()) currency = "?";
            if (risk.trimmed().isEmpty()) risk = "?";
        }
        else qWarning("BondDesc::fromJson WARNING - input jv is not convert to QJsonObject");
    }
    void fromFileLine(const QString &line)
    {
        reset();
        int pos = line.indexOf(".");
        //qDebug()<<QString("pos %1").arg(pos);
        if (pos <= 0 || pos > 5) return;
        bool ok;
        number = line.left(pos).trimmed().toUInt(&ok);
        if (!ok) return;

        QString s = LString::strTrimLeft(line, pos+1).trimmed();
        //qDebug()<<s;
        QStringList list(LString::trimSplitList(s, "/"));
        if (list.count() != filedsCount()) return;

        name = list.at(0);
        country = list.at(1);
        currency = list.at(2);
        risk = list.at(3);
        isin = list.at(4);
        figi = list.at(5);
        uid = list.at(6);
        coupons_year = list.at(7).toInt();
        cur_coupon_size = list.at(8).toFloat();
        start_date = QDate::fromString(list.at(9), userDateMask());
        finish_date = QDate::fromString(list.at(10), userDateMask());
        api_trade = (list.at(11) == "true");
        amortization = (list.at(12) == "true");
        floating_coupon = (list.at(13) == "true");
    }
    QStringList toTableRowData() const
    {
        QStringList row_data;
        if (invalid()) return row_data;

        row_data << name << isin << /*uid <<*/ country << currency;
        row_data << finish_date.toString(userDateMask()) << QString::number(coupons_year) << risk;
        return row_data;
    }

};

// stick params description
struct StockDesc : public InstrumentBase
{
    StockDesc() {reset();}
    StockDesc(const QJsonValue &jv) {fromJson(jv);}

    QString ticker;

    void reset()
    {
        InstrumentBase::reset();
        ticker.clear();
    }
    void fromJson(const QJsonValue &jv)
    {
        reset();
        if (jv.isObject())
        {

        }
    }
};


#endif

