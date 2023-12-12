#include "instrument.h"
#include "lstaticxml.h"
#include "lstring.h"

#include <QDomNode>
#include <QDomElement>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>


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
QDateTime InstrumentBase::dateTimeFromGoogleDT(const QJsonValue &jv_dt)
{
    QDateTime dt;
    dt.setDate(InstrumentBase::dateFromGoogleDT(jv_dt));
    dt.setTime(QTime(1, 1, 1));
    if (jv_dt.isString())
    {
        QString gdt = jv_dt.toString().trimmed();
        int pos = gdt.indexOf("T");
        if (pos > 0)
        {
            gdt = LString::strTrimLeft(gdt, pos+1);
            pos = gdt.indexOf(".");
            if (pos > 0)
            {
                gdt = gdt.left(pos);
                dt.setTime(QTime::fromString(gdt, InstrumentBase::userTimeMask()));
            }
        }
    }
    return dt;
}
float InstrumentBase::floatFromJVBlock(const QJsonValue &jv_block)
{
    float f = -1;
    if (jv_block.isObject())
    {
        const QJsonObject &f_obj = jv_block.toObject();
        const QJsonValue &jv_units = f_obj.value("units");
        float f_units = 0;
        if (jv_units.isString()) f_units = jv_units.toString().toFloat();
        else if (jv_units.isDouble()) f_units = jv_units.toDouble();
        else f_units = -1;

        float f_nano = 0;
        const QJsonValue &jv_nano = f_obj.value("nano");
        if (jv_nano.isString()) f_nano = jv_nano.toString().toFloat();
        else if (jv_nano.isDouble()) f_nano = jv_nano.toDouble();
        else f_nano = -9999;

        while (qAbs(f_nano) > 0.99) f_nano /= double(10);
        f = f_units + f_nano;
    }
    return f;
}
void InstrumentBase::floatToJVBlock(float p, QJsonObject &j_obj)
{
    int i_units = qRound(p);
    if (i_units > p) i_units--;
    int i_nano = QString::number(p, 'f', 2).right(2).toInt();

    qDebug()<<QString("InstrumentBase::floatToJVBlock  i_units=%1   i_nano=%2  need_price=%3").arg(i_units).arg(i_nano).arg(QString::number(p, 'float', 2));
    j_obj["units"] = QString::number(i_units);
    j_obj["nano"] = 10000000*i_nano;
}
void InstrumentBase::fromFileLine(const QString &line)
{
    reset();
    int pos = line.indexOf(".");
    if (pos <= 0 || pos > 5) return;
    bool ok;
    number = line.left(pos).trimmed().toUInt(&ok);
    if (!ok) return;

    QString s = LString::strTrimLeft(line, pos+1).trimmed();
    QStringList list(LString::trimSplitList(s, " / "));

    parseFields(list);

    if (country.toLower().contains("великобрита"))
        country = QString("Великобритания");
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
        currency = getCurrency(j_obj);
        isin = j_obj.value("isin").toString();
        uid = j_obj.value("uid").toString();
        figi = j_obj.value("figi").toString();
        risk = getRisk(j_obj);

        coupons_year = j_obj.value("couponQuantityPerYear").toInt();
        api_trade =  j_obj.value("apiTradeAvailableFlag").toBool();
        amortization =  j_obj.value("amortizationFlag").toBool();
        floating_coupon =  j_obj.value("floatingCouponFlag").toBool();
        start_date = BondDesc::dateFromGoogleDT(j_obj.value("placementDate"));
        finish_date = BondDesc::dateFromGoogleDT(j_obj.value("maturityDate"));
        cur_coupon_size = BondDesc::floatFromJVBlock(j_obj.value("aciValue"));
        nominal = BondDesc::floatFromJVBlock(j_obj.value("nominal"));

        if (country.trimmed().isEmpty()) country = "?";
        if (currency.trimmed().isEmpty()) currency = "?";
        if (risk.trimmed().isEmpty()) risk = "?";
    }
    else qWarning("BondDesc::fromJson WARNING - input jv is not convert to QJsonObject");
}
QString BondDesc::getRisk(const QJsonObject &j_obj) const
{
    QString s = "?";
    const QJsonValue &jv = j_obj.value("riskLevel");
    if (jv.isDouble()) s = QString::number(jv.toDouble());
    else if (jv.isString())
    {
        s = jv.toString().toLower();
        if (s.contains("high")) s = "HIGH";
        else if (s.contains("low")) s = "LOW";
        else s = "MID";
    }
    return s;
}
QString BondDesc::getCurrency(const QJsonObject &j_obj) const
{
    const QJsonValue &jv = j_obj.value("nominal");
    if (jv.isObject())
    {
        const QJsonValue &jv_cur = jv.toObject().value("currency");
        if (jv_cur.isString()) return jv_cur.toString().toUpper();
        else return "ERROR";
    }
    return "?";
}
void BondDesc::parseFields(const QStringList &list)
{
    if (list.count() != filedsCount()) return;

    name = list.at(0);
    country = list.at(1);
    currency = list.at(2);
    risk = list.at(3);
    nominal = list.at(4).toFloat();
    isin = list.at(5);
    figi = list.at(6);
    uid = list.at(7);
    coupons_year = list.at(8).toInt();
    cur_coupon_size = list.at(9).toFloat();
    start_date = QDate::fromString(list.at(10), userDateMask());
    finish_date = QDate::fromString(list.at(11), userDateMask());
    api_trade = (list.at(12) == "true");
    amortization = (list.at(13) == "true");
    floating_coupon = (list.at(14) == "true");
}
void BondDesc::reset()
{
    InstrumentBase::reset();
    isin = risk = QString();
    coupons_year = -1;
    cur_coupon_size = 0;
    start_date = finish_date = QDate();
    amortization = floating_coupon = false;
}
QString BondDesc::toStr() const
{
    QString s(name);
    s = QString("%1 / %2 / %3 / %4 / %5").arg(s).arg(country).arg(currency).arg(risk).arg(nominal);
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

    row_data << name << isin << country << currency << QString::number(nominal, 'f', 1);
    row_data << finish_date.toString(userDateMask()) << QString::number(coupons_year) << risk;
    row_data << QString::number(-1);
    return row_data;
}
int BondDesc::daysToFinish() const
{
    if (!finish_date.isValid()) return -1;
    if (QDate::currentDate() > finish_date) return -1;
    return QDate::currentDate().daysTo(finish_date);
}


//StockDesc
void StockDesc::reset()
{
    InstrumentBase::reset();
    ticker.clear();
    sector.clear();
    in_lot = 1;
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
        figi = j_obj.value("figi").toString();
        uid = j_obj.value("uid").toString();
        ticker = j_obj.value("ticker").toString();
        sector = j_obj.value("sector").toString();
        api_trade =  j_obj.value("apiTradeAvailableFlag").toBool();
        in_lot = j_obj.value("lot").toDouble();

        if (country.trimmed().isEmpty()) country = "?";
        if (sector.trimmed().isEmpty()) sector = "---";
    }
}
void StockDesc::parseFields(const QStringList &list)
{
    if (list.count() != filedsCount()) return;

    name = list.at(0);
    country = list.at(1);
    currency = list.at(2);
    ticker = list.at(3);
    sector = list.at(4);
    figi = list.at(5);
    uid = list.at(6);
    api_trade = (list.at(7) == "true");
    if (list.count() > 8) in_lot = list.at(8).toUInt();
}
QString StockDesc::toStr() const
{
    QString s(name);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(country).arg(currency).arg(ticker);
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(sector).arg(figi).arg(uid);
    s = QString("%1 / %2 / %3").arg(s).arg(api_trade? "true" : "false").arg(in_lot);
    return s;
}
QStringList StockDesc::toTableRowData() const
{
    QStringList row_data;
    if (invalid()) return row_data;

    row_data << name << ticker << country << currency << sector;
    row_data << QString::number(in_lot) << QString::number(-1);
    return row_data;
}



/////////////////////// coupon structures /////////////////////////////////////////
void CouponRecordAbstract::syncData(QDomNode &node, QDomDocument &dom)
{
    if (node.isNull() || invalid()) return;
    QDomNode figi_node = node.namedItem(figi); //ищем существующую ноду с текущим figi
    if (figi_node.isNull())
    {
        //qDebug()<<QString("CouponRecordAbstract::syncData create figi_node <%1>").arg(figi);
        figi_node = dom.createElement(figi); //создаем новую ноду  с текущим figi
        node.appendChild(figi_node);
    }
    toNode(figi_node, dom); //попытка синхронизировать с запись выплат с найденной/созданной родительской figi_node
}
int CouponRecordAbstract::daysTo() const
{
    if (!date.isValid()) return -1;
    if (QDate::currentDate() > date) return -1;
    return QDate::currentDate().daysTo(date);
}
void CouponRecordAbstract::toNode(QDomNode &parent_node, QDomDocument &dom)
{
    //find pay record with this->date
    QDomNode *prev_date_node = NULL;
    QDate max_prev_date = QDate();
    QDomNode *next_date_node = NULL;
    QDate min_next_date = QDate();
    QDomNode pay_node = parent_node.firstChild();
    while (!pay_node.isNull())
    {
        if (pay_node.nodeName() == "pay")
        {
            QDate pay_dt = QDate::fromString(LStaticXML::getStringAttrValue("date", pay_node), InstrumentBase::userDateMask());
            if (pay_dt.isValid())
            {
                if (pay_dt == date) return; //record with date already exists

                if (pay_dt < date)
                {
                    if (!max_prev_date.isValid() || max_prev_date < pay_dt)
                    {
                        max_prev_date = pay_dt;
                        prev_date_node = &pay_node;
                    }
                }
                else
                {
                    if (!min_next_date.isValid() || min_next_date > pay_dt)
                    {
                        min_next_date = pay_dt;
                        next_date_node = &pay_node;
                    }
                }
            }
        }
        pay_node = pay_node.nextSibling();
    }

    //insert/append new  pay record
    QDomElement next_pay_node = dom.createElement("pay");
    setAttrs(next_pay_node);
    if (prev_date_node) parent_node.insertAfter(next_pay_node, *prev_date_node);
    else if (next_date_node) parent_node.insertBefore(next_pay_node, *next_date_node);
    else parent_node.appendChild(next_pay_node);
}
void CouponRecordAbstract::setAttrs(QDomElement &pay_node)
{
    LStaticXML::setAttrNode(pay_node, "date", date.toString(InstrumentBase::userDateMask()), "size", QString::number(size, 'f', 2));
}


//BCoupon
void BCoupon::setAttrs(QDomElement &pay_node)
{
    CouponRecordAbstract::setAttrs(pay_node);
    LStaticXML::setAttrNode(pay_node, "number", QString::number(number), "period", QString::number(period));
}
void BCoupon::fromNode(QDomNode &c_node)
{
    date = QDate::fromString(LStaticXML::getStringAttrValue("date", c_node), InstrumentBase::userDateMask());
    size = LStaticXML::getDoubleAttrValue("size", c_node, -1);
    number = LStaticXML::getIntAttrValue("number", c_node, 9999);
    period = LStaticXML::getIntAttrValue("period", c_node, 0);
}
float BCoupon::daySize() const
{
    if (period > 0) return (size/float(period));
    return -1;
}
void BCoupon::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;

    date = InstrumentBase::dateFromGoogleDT(j_obj.value("couponDate"));
    size = InstrumentBase::floatFromJVBlock(j_obj.value("payOneBond"));
    number = j_obj.value("couponNumber").toString().toUInt();
    figi = j_obj.value("figi").toString();
    period = quint16(j_obj.value("couponPeriod").toDouble());
}


//SDiv
void SDiv::setAttrs(QDomElement &pay_node)
{
    CouponRecordAbstract::setAttrs(pay_node);
    LStaticXML::setAttrNode(pay_node, "yield", QString::number(yield, 'f', 3), "last_price", QString::number(last_price, 'f', 3));
}
void SDiv::fromNode(QDomNode &d_node)
{
    date = QDate::fromString(LStaticXML::getStringAttrValue("date", d_node), InstrumentBase::userDateMask());
    size = LStaticXML::getDoubleAttrValue("size", d_node, -1);
    yield = LStaticXML::getDoubleAttrValue("yield", d_node, -1);
    last_price = LStaticXML::getDoubleAttrValue("last_price", d_node, 0);
}
void SDiv::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;

    date = InstrumentBase::dateFromGoogleDT(j_obj.value("lastBuyDate"));
    size = InstrumentBase::floatFromJVBlock(j_obj.value("dividendNet"));
    yield = InstrumentBase::floatFromJVBlock(j_obj.value("yieldValue"));
    last_price = InstrumentBase::floatFromJVBlock(j_obj.value("closePrice"));
}

//EventOperation
void EventOperation::fromJson(const QJsonValue &jv)
{
    reset();
    if (jv.isObject())
    {
        const QJsonObject &j_obj = jv.toObject();
        uid = j_obj.value("instrumentUid").toString();
        kindByAPIType(j_obj.value("operationType").toString());
        date = InstrumentBase::dateFromGoogleDT(j_obj.value("date"));
        amount = InstrumentBase::floatFromJVBlock(j_obj.value("payment"));
        size = InstrumentBase::floatFromJVBlock(j_obj.value("price"));
        paper_type = j_obj.value("instrumentType").toString();
        currency = j_obj.value("currency").toString().trimmed().toUpper();

        parseTradesJson(j_obj.value("trades"));
    }
    else qWarning("EventOperation::fromJson WARNING - input jv is not convert to QJsonObject");
}
void EventOperation::parseTradesJson(const QJsonValue &jv)
{
    if (kind != etBuy && kind != etSell) return;
    if (!jv.isArray()) return;

    bool ok;
    quint32 quantity = 0;
    const QJsonArray &j_arr = jv.toArray();
    for (int i=0; i<j_arr.count(); i++)
    {
        if (j_arr.at(i).isObject())
        {
            const QJsonObject &j_obj = j_arr.at(i).toObject();
            quantity = j_obj.value("quantity").toString().toUInt(&ok);
            if (ok) n_papers += quantity;
            else qWarning("EventOperation::parseTradesJson WARNING invalid convert to UInt quantity");
        }
    }
}
QString EventOperation::strKind() const
{
    switch (kind)
    {
        case etCanceled:        return QString("CANCELED");
        case etBuy:             return QString("BUY");
        case etSell:            return QString("SELL");
        case etCoupon:          return QString("COUPON");
        case etDiv:             return QString("DIV");
        case etCommission:      return QString("COMMISSION");
        case etTax:             return QString("TAX");
        case etInput:           return QString("INPUT");
        case etOut:             return QString("OUT");
        case etRepayment:       return QString("REPAYMENT");
        default:    break;
    }
    return QString::number(kind);
}
void EventOperation::kindByAPIType(QString api_event_type)
{
   // qDebug()<<QString("EventOperation::kindByAPIType [%1]").arg(api_event_type);
    if (api_event_type == "OPERATION_TYPE_BROKER_FEE") kind = etCommission;
    else if (api_event_type == "OPERATION_TYPE_BUY") kind = etBuy;
    else if (api_event_type == "OPERATION_TYPE_SELL") kind = etSell;
    else if (api_event_type == "OPERATION_TYPE_TAX") kind = etTax;
    else if (api_event_type == "OPERATION_TYPE_BOND_TAX") kind = etTax;
    else if (api_event_type == "OPERATION_TYPE_DIVIDEND_TAX") kind = etTax;
    else if (api_event_type == "OPERATION_TYPE_DIVIDEND") kind = etDiv;
    else if (api_event_type == "OPERATION_TYPE_OUTPUT") kind = etOut;
    else if (api_event_type == "OPERATION_TYPE_INP_MULTI") kind = etInput;
    else if (api_event_type == "OPERATION_TYPE_INPUT") kind = etInput;
    else if (api_event_type == "OPERATION_TYPE_BOND_REPAYMENT_FULL") kind = etRepayment;
    else if (api_event_type == "OPERATION_TYPE_COUPON") kind = etCoupon;

}


//OrderData
void OrderData::fromJson(const QJsonValue &jv)
{
    reset();
    if (jv.isObject())
    {
        const QJsonObject &j_obj = jv.toObject();
        uid = j_obj.value("instrumentUid").toString();
        order_id = j_obj.value("orderId").toString();
        lots.first = j_obj.value("lotsRequested").toString().toUInt();
        lots.second = j_obj.value("lotsExecuted").toString().toUInt();
        time = InstrumentBase::dateTimeFromGoogleDT(j_obj.value("orderDate"));

        type = j_obj.value("direction").toString();
        type.remove("ORDER_DIRECTION");
        type = QString("%1/%2").arg(type).arg(j_obj.value("orderType").toString());
        type.remove("ORDER_TYPE");
        type.remove("_");

        price = InstrumentBase::floatFromJVBlock(j_obj.value("initialSecurityPrice"));
        currency = j_obj.value("currency").toString().trimmed().toUpper();
  }
    else qWarning("OrderData::fromJson WARNING - input jv is not convert to QJsonObject");
}
QString OrderData::strLots() const
{
    return QString("%1 / %2").arg(lots.first).arg(lots.second);
}
void OrderData::reset()
{
    type = QString("?");
    time = QDateTime();
    order_id.clear();
    currency.clear();
    uid.clear();
    price=-1;
    lots.first=lots.second=0;
}
bool OrderData::invalid() const
{
    return (type==QString("?") || !time.isValid() || uid.isEmpty() || order_id.isEmpty() || price<=0);
}



//StopOrderData
void StopOrderData::fromJson(const QJsonValue &jv)
{
    reset();
    if (jv.isObject())
    {
        const QJsonObject &j_obj = jv.toObject();
        uid = j_obj.value("instrumentUid").toString();
        order_id = j_obj.value("stopOrderId").toString();
        lots.first = j_obj.value("lotsRequested").toString().toUInt();
        time = InstrumentBase::dateTimeFromGoogleDT(j_obj.value("createDate"));
        currency = j_obj.value("currency").toString().trimmed().toUpper();
        price = InstrumentBase::floatFromJVBlock(j_obj.value("stopPrice"));

        type = j_obj.value("direction").toString();
        type.remove("STOP_ORDER_DIRECTION");
        type = QString("%1/%2").arg(type).arg(j_obj.value("orderType").toString());
        type.remove("STOP_ORDER_TYPE");
        type.remove("_");
  }
    else qWarning("StopOrderData::fromJson WARNING - input jv is not convert to QJsonObject");
}
QString StopOrderData::strLots() const
{
    return QString("%1").arg(lots.first);
}


//PlaceOrderData
void PlaceOrderData::reset()
{
    uid.clear();
    kind = "sell"; //buy/sell
    is_stop = false;
    price = -1;
    lots = 1;
}
bool PlaceOrderData::invalid() const
{
    if (kind != "buy" && kind != "sell" && kind != "cancel") return true;
    if (uid.isEmpty()) return true;
    if (!isCancel()) return (price<=0 || lots<1);
    return false;
}

