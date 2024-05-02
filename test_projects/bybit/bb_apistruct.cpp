#include "bb_apistruct.h"
#include "lhttp_types.h"
#include "lstring.h"
#include "apiconfig.h"

#include <QStringList>
#include <QJsonValue>
#include <QJsonObject>
#include <QDebug>

#define PRICE_PRECISION     4
#define USDT_SYMBOL         QString("USDT")

//BB_APIReqParams
QString BB_APIReqParams::fullUri() const
{
    QString s;
    if (!invalid())
    {
        s = uri;
        QString ps(paramsLine().trimmed());
        if (!ps.isEmpty()) s = QString("%1?%2").arg(s).arg(ps);
    }
    return s;
}
QString BB_APIReqParams::paramsLine() const
{
    if (invalid() || params.isEmpty())  return QString();

    QString s;
    QStringList keys(params.keys());
    foreach (const QString &v, keys)
        s.append(QString("%1=%2&").arg(v).arg(params.value(v)));
    return s.left(s.length()-1);
}
bool BB_APIReqParams::invalid() const
{
    if (metod != hrmGet && metod != hrmPost) return true;
    return (name.isEmpty() || uri.isEmpty());
}
QString BB_APIReqParams::toStr() const
{
    QString s("BB_APIReqParams: ");
    s = QString("%1 name[%2] metod[%3] params[%4] uri[%5]  req_type[%6]").arg(s).arg(name).arg(metod).arg(params.count()).arg(uri).arg(req_type);
    s = QString("%1 validity[%2]").arg(s).arg(invalid()?"FAULT":"OK");
    return s;
}
QString BB_APIReqParams::strReqTypeByType(int t, QString s_extra)
{
    QString s;
    switch (t)
    {
        case rtCandles:     {s = "GET_CANDLES"; break;}
        case rtPositions:   {s = "GET_POSITIONS"; break;}
        case rtSpotAssets:  {s = "GET_SPOT_ASSETS"; break;}
        case rtSportOrders: {s = "GET_SPOT_ORDERS"; break;}
        case rtOrders:      {s = "GET_ORDERS"; break;}
        case rtHistory:     {s = "GET_HISTORY"; break;}
        case rtBag:         {s = "GET_WALLET"; break;}
        case rtFundRate:    {s = "GET_FUND_RATES"; break;}
        default: return "???";
    }
    return s_extra.isEmpty() ? s : QString("%1(%2)").arg(s).arg(s_extra);
}

//////////BB_BagState/////////////
float BB_BagState::sumFreezed() const
{
    float sum = freezed_order + freezed_pos;
    if (sum < 0) return -1;
    return sum;
}

// BB_HistoryState
void BB_HistoryState::reset()
{
    closed_pos = closed_orders = canceled_orders = 0;
    paid_commission = total_pnl = 0;
}


//BB_HistoryRecordBase
void BB_HistoryRecordBase::reset()
{
    uid.clear();
    ticker.clear();
    lot_size = 0;
    action.clear();
    type.clear();
    status.clear();
}
void BB_HistoryRecordBase::fromFileLine(const QString &line)
{
    reset();
    QStringList list(LString::trimSplitList(line, " / "));
    if (list.count() != filedsCount())
    {
        qWarning()<<QString("BB_HistoryRecordBase::fromFileLine WARNING - list.count(%1) != filedsCount(%2)").arg(list.count()).arg(filedsCount());
        return;
    }

    bool ok;
    int i = 1;
    uid = list.at(i).trimmed(); i++;
    ticker = list.at(i).trimmed(); i++;
    action = list.at(i).trimmed(); i++;
    lot_size = list.at(i).trimmed().toFloat(&ok);
    if (!ok)
    {
        lot_size = -1;
        qWarning()<<QString("BB_HistoryRecordBase::fromFileLine WARNING - invalid lot value [%1]").arg(list.at(i));
    }

    if (!invalid())
    {
        parseSplitedFileLine(list);
        status = list.last();
    }
}

// BB_HistoryPos
void BB_HistoryPos::reset()
{
    BB_HistoryRecordBase::reset();

    closed_time = QDateTime();
    leverage = 0;
    open_price = closed_price = total_result = -1;
    //trigger.clear();
}
QString BB_HistoryPos::toFileLine() const
{
    QString s(closed_time.toString(BB_APIReqParams::userDateTimeMask()));
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(uid).arg(ticker).arg(action);
    s = QString("%1 / %2").arg(s).arg(QString::number(lot_size, 'f', PRICE_PRECISION));
    s = QString("%1 / %2").arg(s).arg(leverage);
    s = QString("%1 / %2").arg(s).arg(QString::number(open_price, 'f', PRICE_PRECISION));
    s = QString("%1 / %2").arg(s).arg(QString::number(closed_price, 'f', PRICE_PRECISION));
    s = QString("%1 / %2").arg(s).arg(QString::number(total_result, 'f', PRICE_PRECISION));
    s = QString("%1 / %2 / %3").arg(s).arg(type).arg(status);
    return QString("%1 \n").arg(s);
}
void BB_HistoryPos::parseSplitedFileLine(const QStringList &list)
{
    closed_time = QDateTime::fromString(list.first(), BB_APIReqParams::userDateTimeMask());

    bool ok;
    int i = 5;
    leverage = list.at(i).trimmed().toUInt(&ok); i++;
    if (!ok)
    {
        leverage = 99;
        qWarning()<<QString("BB_HistoryPos::parseSplitedFileLine WARNING - invalid leverage value [%1]").arg(list.at(i-1));
    }
    open_price = list.at(i).trimmed().toFloat(&ok); i++;
    if (!ok)
    {
        open_price = -2;
        qWarning()<<QString("BB_HistoryPos::parseSplitedFileLine WARNING - invalid open_price value [%1]").arg(list.at(i-1));
    }
    closed_price = list.at(i).trimmed().toFloat(&ok); i++;
    if (!ok)
    {
        closed_price = -2;
        qWarning()<<QString("BB_HistoryPos::parseSplitedFileLine WARNING - invalid closed_price value [%1]").arg(list.at(i-1));
    }
    total_result = list.at(i).trimmed().toFloat(&ok); i++;
    if (!ok)
    {
        total_result = -2;
        qWarning()<<QString("BB_HistoryPos::parseSplitedFileLine WARNING - invalid total_result value [%1]").arg(list.at(i-1));
    }
    //trigger = list.at(i).trimmed(); i++;
    type = list.at(i).trimmed(); i++;
}
void BB_HistoryPos::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;

    closed_time.setMSecsSinceEpoch(j_obj.value("updatedTime").toString().toLong());

    uid = j_obj.value("orderId").toString();
    ticker = j_obj.value("symbol").toString();
    type = j_obj.value("orderType").toString();
    status = j_obj.value("execType").toString();

    QString act(j_obj.value("side").toString().trimmed().toUpper());
    if (act == "SELL") action = "LONG";
    else if (act == "BUY") action = "SHORT";

    lot_size = j_obj.value("qty").toString().toFloat();
    leverage = j_obj.value("leverage").toString().toUInt();
    total_result = j_obj.value("closedPnl").toString().toFloat();
    open_price = j_obj.value("avgEntryPrice").toString().toFloat();
    closed_price = j_obj.value("avgExitPrice").toString().toFloat();
}
QStringList BB_HistoryPos::toTableRowData() const
{
    QStringList list;
    list.append(closed_time.toString(BB_APIReqParams::userDateTimeMask()));
    list << ticker << action;
    list << QString::number(lot_size, 'f', 2) << QString::number(leverage);
    list << priceInfo() << QString::number(paidFee(), 'f', 3);
    list << QString::number(total_result, 'f', 2) << type << status;
    return list;
}
float BB_HistoryPos::paidFee() const
{
    float fee = -1;
    if (isLong()) fee = total_result - lot_size*dPrice();
    else if (isShort()) fee = total_result + lot_size*dPrice();
    return fee;
}
QString BB_HistoryPos::priceInfo() const
{
    float deviation = -1;
    if (isLong()) deviation = float(100)*dPrice()/open_price;
    else if (isShort()) deviation = float(-1*100)*dPrice()/open_price;
    return QString("%1 / %2 (%3%)").arg(QString::number(open_price, 'f', 2)).arg(QString::number(closed_price, 'f', 2)).arg(QString::number(deviation, 'f', 1));
}
QStringList BB_HistoryPos::tableHeaders()
{
    QStringList list;
    list << "Close time" << "Ticker" << "Action" << "Volume" << "Leverage";
    list << "Price" << "Commision" << "Total result" << "Order type" << "Trigger";
    return list;
}

// BB_HistoryOrder
void BB_HistoryOrder::reset()
{
    BB_HistoryRecordBase::reset();

    create_time = QDateTime();
    price = -1;
    is_leverage = false;
}
QString BB_HistoryOrder::toFileLine() const
{
    QString s(create_time.toString(BB_APIReqParams::userDateTimeMask()));
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(uid).arg(ticker).arg(action);
    s = QString("%1 / %2").arg(s).arg(QString::number(lot_size, 'f', PRICE_PRECISION));
    s = QString("%1 / %2").arg(s).arg(is_leverage ? "true" : "false");
    s = QString("%1 / %2").arg(s).arg(QString::number(price, 'f', PRICE_PRECISION));
    s = QString("%1 / %2 / %3").arg(s).arg(type).arg(status);
    return QString("%1 \n").arg(s);
}
void BB_HistoryOrder::parseSplitedFileLine(const QStringList &list)
{
    create_time = QDateTime::fromString(list.first(), BB_APIReqParams::userDateTimeMask());

    bool ok;
    int i = 5;
    is_leverage = (list.at(i).trimmed().toLower() == "true"); i++;
    price = list.at(i).trimmed().toFloat(&ok); i++;
    if (!ok)
    {
        price = -2;
        qWarning()<<QString("BB_HistoryPos::parseSplitedFileLine WARNING - invalid price value [%1]").arg(list.at(i-1));
    }
    type = list.at(i).trimmed(); i++;
}
QStringList BB_HistoryOrder::tableHeaders()
{
    QStringList list;
    list << "Create time" << "Ticker" << "Action" << "Volume" << "Has leverage";
    list << "Price" << "Order type" << "Trigger";
    return list;
}
QStringList BB_HistoryOrder::toTableRowData() const
{
    QStringList list;
    list.append(create_time.toString(BB_APIReqParams::userDateTimeMask()));
    list << ticker << action << QString::number(lot_size, 'f', 2);
    list << QString("%1").arg(is_leverage ? "YES" : "NO");
    list << QString::number(price, 'f', 2) << type << status;
    return list;
}
void BB_HistoryOrder::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;


    create_time.setMSecsSinceEpoch(j_obj.value("createdTime").toString().toLong());
    uid = j_obj.value("orderId").toString();
    ticker = j_obj.value("symbol").toString();
    lot_size = j_obj.value("qty").toString().toFloat();
    action = j_obj.value("side").toString().toUpper().trimmed();
    status = j_obj.value("orderStatus").toString();

    type = j_obj.value("stopOrderType").toString().toUpper().trimmed();
    if (type.isEmpty()) type = j_obj.value("orderType").toString().toUpper().trimmed();

    if (isLimit()) price = j_obj.value("price").toString().toFloat();
    else if (isStop()) price = j_obj.value("triggerPrice").toString().toFloat();

    //qDebug()<<QString("isLeverage [%1]").arg(j_obj.value("isLeverage").toString());
    is_leverage = (j_obj.value("isLeverage").toString() == "0");

}


// BB_HistorySpot
void BB_HistorySpot::reset()
{
    BB_HistoryRecordBase::reset();

    triger_time = QDateTime();
    price = -1;
    fee = 0;
    fee_ticker.clear();
}
QStringList BB_HistorySpot::tableHeaders()
{
    QStringList list;
    list << "Trigger time" << "Ticker" << "Action" << "Price" << "Volume" << "Fee" << "Fee ticker";
    list << "Order type" << "USD size" << "Result";
    return list;
}
QString BB_HistorySpot::toFileLine() const
{
    QString s(triger_time.toString(BB_APIReqParams::userDateTimeMask()));
    s = QString("%1 / %2 / %3 / %4").arg(s).arg(uid).arg(ticker).arg(action);
    s = QString("%1 / %2").arg(s).arg(QString::number(lot_size, 'f', PRICE_PRECISION));
    s = QString("%1 / %2").arg(s).arg(QString::number(price, 'f', PRICE_PRECISION));
    s = QString("%1 / %2").arg(s).arg(QString::number(fee, 'f', PRICE_PRECISION));
    s = QString("%1 / %2").arg(s).arg(fee_ticker);
    s = QString("%1 / %2 / EXEC").arg(s).arg(type);

    return QString("%1 \n").arg(s);
}
void BB_HistorySpot::parseSplitedFileLine(const QStringList &list)
{
    triger_time = QDateTime::fromString(list.first(), BB_APIReqParams::userDateTimeMask());

    bool ok;
    int i = 5;
    price = list.at(i).trimmed().toFloat(&ok); i++;
    if (!ok)
    {
        price = -2;
        qWarning()<<QString("BB_HistorySpot::parseSplitedFileLine WARNING - invalid price value [%1]").arg(list.at(i-1));
    }
    fee = list.at(i).trimmed().toFloat(&ok); i++;
    if (!ok)
    {
        fee = -2;
        qWarning()<<QString("BB_HistorySpot::parseSplitedFileLine WARNING - invalid fee value [%1]").arg(list.at(i-1));
    }
    fee_ticker = list.at(i).trimmed(); i++;
    type = list.at(i).trimmed(); i++;
}
QStringList BB_HistorySpot::toTableRowData() const
{
    QStringList list;
    list.append(triger_time.toString(BB_APIReqParams::userDateTimeMask()));
    list << ticker << action << QString::number(price, 'f', 2) << QString::number(lot_size, 'f', 2);
    list << QString("%1 (%2%)").arg(QString::number(fee, 'f', 3)).arg(QString::number(pFee(), 'f', 2));
    list << fee_ticker;
    list << type << QString::number(usdSize(), 'f', 1) << resultDeal();
    return list;
}
void BB_HistorySpot::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;

    triger_time.setMSecsSinceEpoch(j_obj.value("execTime").toString().toLong());
    uid = j_obj.value("orderId").toString();
    fee_ticker = j_obj.value("feeCurrency").toString();
    ticker = j_obj.value("symbol").toString();
    int pos = ticker.indexOf(USDT_SYMBOL);
    if (pos > 0) ticker = ticker.left(pos);

    lot_size = j_obj.value("execQty").toString().toFloat();
    action = j_obj.value("side").toString().toUpper().trimmed();
    price = j_obj.value("execPrice").toString().toFloat();
    fee = j_obj.value("execFee").toString().toFloat();
    type = j_obj.value("orderType").toString();

}
float BB_HistorySpot::pFee() const
{
    if (invalid()) return -9999;
    if (fee_ticker == ticker) return (float(100)*fee/lot_size);
    return (float(100)*fee/usdSize());
}
float BB_HistorySpot::usdSize() const
{
    if (invalid()) return -9999;
    return price*lot_size;
}
QString BB_HistorySpot::resultDeal() const
{
    if (invalid()) return "??";
    float r = lot_size;
    if (isBuy()) r = lot_size - fee;
    else if (isSell()) r = usdSize() - fee;
    return QString::number(r, 'f', 2);
}

