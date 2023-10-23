#include "instrument.h"

//InstrumentBase
QDate InstrumentBase::dateFromGoogleDT(const QJsonValue &jv_dt)
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
float InstrumentBase::floatFromJVBlock(const QJsonValue &jv_block)
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


//BondDesc
void BondDesc::fromJson(const QJsonValue &jv)
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
void BondDesc::fromFileLine(const QString &line)
{
    reset();
    int pos = line.indexOf(".");
    if (pos <= 0 || pos > 5) return;
    bool ok;
    number = line.left(pos).trimmed().toUInt(&ok);
    if (!ok) return;

    QString s = LString::strTrimLeft(line, pos+1).trimmed();
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
void BondDesc::reset()
{
    InstrumentBase::reset();
    isin = figi = risk = QString();
    coupons_year = -1;
    cur_coupon_size = 0;
    start_date = finish_date = QDate();
    amortization = floating_coupon = false;
}
QString BondDesc::toStr() const
{
    QString s(name);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(country).arg(currency).arg(risk);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(isin).arg(figi).arg(uid);
    s = QString("%1 / %2 / %3").arg(s).arg(coupons_year).arg(QString::number(cur_coupon_size, 'f', 1));
    s = QString("%1 / %2 / %3").arg(s).arg(start_date.toString(userDateMask())).arg(finish_date.toString(userDateMask()));
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(api_trade?"true":"false").arg(amortization?"true":"false").arg(floating_coupon?"true":"false");
    return s;
}
QStringList BondDesc::toTableRowData() const
{
    QStringList row_data;
    if (invalid()) return row_data;

    row_data << name << isin << /*uid <<*/ country << currency;
    row_data << finish_date.toString(userDateMask()) << QString::number(coupons_year) << risk;
    return row_data;
}


//StockDesc
void StockDesc::reset()
{
    InstrumentBase::reset();
    ticker.clear();
    sector.clear();
}
void StockDesc::fromJson(const QJsonValue &jv)
{
    reset();
    if (jv.isObject())
    {
        const QJsonObject &j_obj = jv.toObject();
        name = j_obj.value("name").toString();
        country = j_obj.value("countryOfRiskName").toString();
        currency = j_obj.value("currency").toString();
        uid = j_obj.value("uid").toString();
        ticker = j_obj.value("ticker").toString();
        sector = j_obj.value("sector").toString();
        api_trade =  j_obj.value("apiTradeAvailableFlag").toBool();
    }
}
void StockDesc::fromFileLine(const QString &line)
{
    reset();
    int pos = line.indexOf(".");
    if (pos <= 0 || pos > 5) return;
    bool ok;
    number = line.left(pos).trimmed().toUInt(&ok);
    if (!ok) return;

    QString s = LString::strTrimLeft(line, pos+1).trimmed();
    QStringList list(LString::trimSplitList(s, "/"));
    if (list.count() != filedsCount()) return;

    name = list.at(0);
    country = list.at(1);
    currency = list.at(2);
    ticker = list.at(3);
    sector = list.at(4);
    uid = list.at(5);
    api_trade = (list.at(6) == "true");
}
QString StockDesc::toStr() const
{
    QString s(name);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(country).arg(currency).arg(ticker);
    s = QString("%1 / %2 / %3").arg(s).arg(sector).arg(uid);
    s = QString("%1 / %2").arg(s).arg(api_trade?"true":"false");
    return s;
}
QStringList StockDesc::toTableRowData() const
{
    QStringList row_data;
    if (invalid()) return row_data;

    row_data << name << ticker << country << currency << sector;
    return row_data;
}

