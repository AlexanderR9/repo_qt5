#include "ug_apistruct.h"
#include "lhttp_types.h"
#include "lstring.h"
#include "lfile.h"
#include "subcommonsettings.h"

#include <QDir>
#include <QStringList>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonArray>
#include <QtMath>

#include <QDebug>
#include <QDateTime>

#define APP_DATA_FOLDER     QString("data")



//UG_APIReqParams
QString UG_APIReqParams::strReqTypeByType(int t, QString s_extra)
{
    QString s;
    switch (t)
    {
        case rtPools:           {s = "GET_POOL_DATA"; break;}
        case rtTokens:          {s = "GET_TOKEN_DATA"; break;}
        case rtJsonView:        {s = "FREE_QUERY"; break;}
        case rtDaysData:        {s = "GET_DAYS_DATA"; break;}
        case rtPositions:       {s = "GET_POS_DATA"; break;}

        default: return "???";
    }
    return s_extra.isEmpty() ? s : QString("%1(%2)").arg(s).arg(s_extra);
}

//UG_PoolDayData
void UG_PoolDayData::reset()
{
    tvl = price = -1;
    feesSize = volume = 0;
    date = 0;
}
bool UG_PoolDayData::invalid() const
{
    if (date < 1000000) return true;
    if (tvl <= 0 || price <= 0) return true;
    return false;
}
void UG_PoolDayData::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;

    tvl = j_obj.value("tvlUSD").toString().toDouble();
    feesSize = j_obj.value("feesUSD").toString().toDouble();
    price = j_obj.value("close").toString().toDouble();
    volume = j_obj.value("volumeUSD").toString().toDouble();
    date = uint(j_obj.value("date").toDouble());

//    fee = fee/float(10000);
}
void UG_PoolDayData::toTableRow(QStringList &list) const
{
    QDateTime dt = QDateTime::fromSecsSinceEpoch(date);
    //int days = dt.daysTo(QDateTime::currentDateTime());
    float fv = 1000000;

    list.clear();
    list << dt.toString(UG_APIReqParams::userDateMask()) << QString::number(price, 'f', 4);
    list << QString::number(tvl/fv, 'f', 2) << QString::number(volume, 'f', 1) << QString::number(feesSize, 'f', 1);

    double f_th = -1;
    if (tvl > 0) f_th = feesSize*1000/tvl;
    list << QString::number(f_th, 'f', 2);
    list << "-" << "-";
}
QString UG_PoolDayData::toStr() const
{
    QString s("UG_PoolDayData: ");
    s = QString("%1 date[%2] tvl[%3] fee[%4]").arg(s).arg(date).arg(QString::number(tvl, 'f', 1)).arg(QString::number(feesSize, 'f', 1));
    s = QString("%1 volume[%2]  price[%3]").arg(s).arg(QString::number(volume, 'f', 1)).arg(QString::number(price, 'f', 4));
    return s;
}

//UG_PosInfo
void UG_PosInfo::reset()
{
    id = "?";
    pool.reset();
    //chain.clear();
    deposited.first = deposited.second = 0;
    //collected.first = collected.second = 0;
    collectedFees.first = collectedFees.second = 0;
    withdrawn.first = withdrawn.second = 0;
    ts = 0;
    digit.first = digit.second = 1;
    tick_range.first = tick_range.second = -1;
    liquidity = 0;
    cur_tick = 0;
    token0Price = -1;
}
bool UG_PosInfo::invalid() const
{
    if (id.length() < 6 || chain.trimmed().isEmpty() || ts == 0) return true;
    if (pool.token0.trimmed().isEmpty() || pool.token1.trimmed().isEmpty()) return true;
    if (pool.tvl <= 0 || pool.fee <= 0) return true;
    return (tick_range.first == tick_range.second);
}
void UG_PosInfo::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;

    id = j_obj.value("id").toString();
    //collected.first = j_obj.value("collectedToken0").toString().toDouble();
    //collected.second = j_obj.value("collectedToken1").toString().toDouble();
    collectedFees.first = j_obj.value("collectedFeesToken0").toString().toDouble();
    collectedFees.second = j_obj.value("collectedFeesToken1").toString().toDouble();
    deposited.first = j_obj.value("depositedToken0").toString().toDouble();
    deposited.second = j_obj.value("depositedToken1").toString().toDouble();
    withdrawn.first =  j_obj.value("withdrawnToken0").toString().toDouble();
    withdrawn.second =  j_obj.value("withdrawnToken1").toString().toDouble();
    liquidity =  j_obj.value("liquidity").toString().toLong();

    pool.fromJson(j_obj.value("pool").toObject());

    QJsonObject j_transaction = j_obj.value("transaction").toObject();
    if (j_transaction.isEmpty())
    {
        qWarning("UG_PosInfo::fromJson - WARNING j_transaction.isEmpty()");
        return;
    }
    ts = j_transaction.value("timestamp").toString().toUInt();

    QJsonArray j_mint = j_transaction.value("mints").toArray();
    if (!j_mint.isEmpty())
    {
        tick_range.first = j_mint.at(0).toObject().value("tickLower").toString().toInt();
        tick_range.second = j_mint.at(0).toObject().value("tickUpper").toString().toInt();
        cur_tick = j_obj.value("pool").toObject().value("tick").toString().toInt();
        token0Price = j_obj.value("pool").toObject().value("token0Price").toString().toDouble();
        if (token0Price > 0) token0Price = 1/token0Price;
    }
    else qWarning("WARNING: mints node not found");

    calcDigit();
}
void UG_PosInfo::calcDigit()
{
    if (invalid()) return;
    if (pool.token0.contains("ETH")) digit.first = 4;
    if (pool.token1.contains("ETH")) digit.second = 4;
    if (pool.token0.contains("BNB")) digit.first = 3;
    if (pool.token1.contains("BNB")) digit.second = 3;
    if (pool.token0.contains("AAVE")) digit.first = 3;
    if (pool.token1.contains("AAVE")) digit.second = 3;
    if (pool.token0.contains("LINK")) digit.first = 2;
    if (pool.token1.contains("LINK")) digit.second = 2;
}
void UG_PosInfo::toTableRow(QStringList &list) const
{
    QDateTime dt = QDateTime::fromSecsSinceEpoch(ts);

    list.clear();
    list << id << dt.toString(UG_APIReqParams::userDateTimeMask()) << poolParams() << chain;
    list << QString("%1k").arg(QString::number(pool.tvl/float(1000), 'f', 1));
    list << QString("%1 / %2").arg(QString::number(deposited.first, 'f', digit.first)).arg(QString::number(deposited.second, 'f', digit.second));
    //list << QString("%1 / %2").arg(QString::number(collected.first, 'f', digit.first)).arg(QString::number(collected.second, 'f', digit.second));
    list << QString("%1 / %2").arg(QString::number(withdrawn.first, 'f', digit.first)).arg(QString::number(withdrawn.second, 'f', digit.second));
    list << QString("%1 / %2").arg(QString::number(collectedFees.first, 'f', digit.first)).arg(QString::number(collectedFees.second, 'f', digit.second));
    list << QString("%1 / %2 (cur: %3)").arg(tick_range.first).arg(tick_range.second).arg(cur_tick);
    list << priceRange();

}
QString UG_PosInfo::toStr() const
{
    QString s = QString("UG_PosInfo(%1): ").arg(id);
    s = QString("%1 ts[%2] chain[%3] pool[%4]").arg(s).arg(ts).arg(chain).arg(poolParams());
    s = QString("%1 tick_range[%2; %3]").arg(s).arg(tick_range.first).arg(tick_range.second);
    s = QString("%1 liquidity[%2]  token0Price[%3]").arg(s).arg(liquidity).arg(token0Price);
    return s;
}
QString UG_PosInfo::poolParams() const
{
    QString pp = QString("%1 / %2 /").arg(pool.token0).arg(pool.token1);
    //pp.append(QString(" %1% / %2k").arg(QString::number(pool.fee, 'f', 2)).arg(QString::number(pool.tvl/float(1000), 'f', 1)));
    pp.append(QString(" %1%").arg(QString::number(pool.fee, 'f', 2)));
    return pp;
}
QString UG_PosInfo::priceRange() const
{
    double p1, p2;
    calcPricesByTicks(p1, p2);
    if (p1 < 0 || p2 < 0) return QString("??");

    p1 = toStablePrice(p1);
    p2 = toStablePrice(p2);
    double p_cur = token0Price;
    double min = qMin(p1, p2);
    if (min < p1) {p2 = p1; p1 = min; p_cur = 1/token0Price;}
    qint8 prec = 1;
    if (min < 0.01) prec = 6;
    else if (min < 1.1) prec = 4;
    else if (min < 5) prec = 3;
    else if (min < 50) prec = 2;


    QString s = QString("[%1; %2]").arg(QString::number(p1, 'f', prec)).arg(QString::number(p2, 'f', prec));
    s = QString("%1 cur=%2").arg(s).arg(QString::number(p_cur, 'f', prec));
    return s;
}
double UG_PosInfo::toStablePrice(const double &p0) const
{
    if (pool.token1.trimmed().toUpper() == "USDT") return p0;
    if (pool.token0.trimmed().toUpper() == "USDT") return (1/p0);
    if (pool.token1.trimmed().toUpper() == "USDC") return p0;
    if (pool.token0.trimmed().toUpper() == "USDC") return (1/p0);
    return p0;
}
void UG_PosInfo::calcPricesByTicks(double &p1, double &p2) const
{
    p1 = p2 = -1;
    if (invalid()) return;

    int dec0 = sub_commonSettings.tokenDecimal(pool.token0, chain);
    int dec1 = sub_commonSettings.tokenDecimal(pool.token1, chain);
    if (dec0 < 0 || dec1< 0) return;

    double a = qPow(sub_commonSettings.tickKwant(), tick_range.first);
    double b = qPow(10, (dec1 - dec0));
    p1 = a/b;

    a = qPow(sub_commonSettings.tickKwant(), tick_range.second);
    p2 = a/b;
}
bool UG_PosInfo::isOut() const
{
    double p1, p2;
    calcPricesByTicks(p1, p2);
    if (p1 < 0 || p2 < 0) return false;

    return (token0Price < p1 || token0Price > p2);
}



//UG_PoolInfo
void UG_PoolInfo::reset()
{
    id = "?";
    tvl = -1;
    token0.clear();
    token1.clear();
    token0_id.clear();
    token1_id.clear();
    fee = volume_all = 0;
    ts = 0;
}
bool UG_PoolInfo::invalid() const
{
    if (id.length() < 10) return true;
    if (token0.trimmed().isEmpty() || token1.trimmed().isEmpty()) return true;
    if (token0_id.trimmed().isEmpty() || token1_id.trimmed().isEmpty()) return true;
    if (token0.trimmed().length()>5 || token1.trimmed().length()>5) return true;
    if (tvl <= 0 || fee <= 0 || ts == 0) return true;
    return false;
}
void UG_PoolInfo::fromJson(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;

    id = j_obj.value("id").toString();
    fee = j_obj.value("feeTier").toString().toDouble();
    tvl = j_obj.value("totalValueLockedUSD").toString().toDouble();
    volume_all = j_obj.value("volumeUSD").toString().toDouble();
    ts = j_obj.value("createdAtTimestamp").toString().toUInt();

    token0 = j_obj.value("token0").toObject().value("symbol").toString().trimmed().toUpper();
    token0_id = j_obj.value("token0").toObject().value("id").toString().trimmed();
    token1 = j_obj.value("token1").toObject().value("symbol").toString().trimmed().toUpper();
    token1_id = j_obj.value("token1").toObject().value("id").toString().trimmed();
    fee = fee/float(10000);
}
QString UG_PoolInfo::toStr() const
{
    QString s("PoolInfo: ");
    s = QString("%1 id[%2] tvl[%3] fee[%4]").arg(s).arg(id).arg(QString::number(tvl, 'f', 1)).arg(QString::number(fee, 'f', 2));
    s = QString("%1 volume_all[%2]").arg(s).arg(QString::number(volume_all, 'f', 1));
    s = QString("%1 tokens[%2/%3]").arg(s).arg(token0).arg(token1);
    s = QString("%1 t_creation[%2]").arg(s).arg(ts);

    return s;
}
void UG_PoolInfo::toTableRow(QStringList &list) const
{
    QDateTime dt = QDateTime::fromSecsSinceEpoch(ts);
    int days = dt.daysTo(QDateTime::currentDateTime());

    float fv = 1000000;
    list.clear();
    //list << id << QString::number(tvl, 'f', 1) << QString::number(volume_all, 'f', 1);
    list << id << QString::number(tvl/fv, 'f', 2) << QString::number(volume_all/fv, 'f', 2);
    list << token0 << token1 << QString("%1%").arg(QString::number(fee, 'f', 2));
    list << QString("%1 (%2 days)").arg(dt.toString(UG_APIReqParams::userDateMask())).arg(days);

}
QString UG_PoolInfo::toFileLine() const
{
    QString fline = QString("%1 / %2").arg(id).arg(ts);
    fline = QString("%1 / %2 / %3 / %4 / %5").arg(fline).arg(token0).arg(token0_id).arg(token1).arg(token1_id);
    fline = QString("%1 / %2").arg(fline).arg(QString::number(tvl, 'f', 2));
    fline = QString("%1 / %2").arg(fline).arg(QString::number(volume_all, 'f', 2));
    fline = QString("%1 / %2").arg(fline).arg(QString::number(fee, 'f', 2));
    return fline;
}
void UG_PoolInfo::fromFileLine(const QString &fline)
{
    QStringList list = LString::trimSplitList(fline.trimmed(), "/");
    if (list.count() != 9) {qWarning()<<QString("UG_PoolInfo WARNING invalid fline [%1]").arg(fline); return;}

    int i = 0;
    id = list.at(i); i++;
    ts = list.at(i).toUInt(); i++;
    token0 = list.at(i); i++;
    token0_id = list.at(i); i++;
    token1 = list.at(i); i++;
    token1_id = list.at(i); i++;
    tvl = list.at(i).toDouble(); i++;
    volume_all = list.at(i).toDouble(); i++;
    fee = list.at(i).toDouble(); i++;
}


//UG_TokenInfo
UG_TokenInfo::UG_TokenInfo(QString a, QString t, QString c)
    :address(a.trimmed()),
      ticker(t.trimmed()),
      chain(c.trimmed())
{
    name = "---";
    collected_fees = tvl = total_supply = 0;
    decimal = 0;
}
void UG_TokenInfo::reset()
{
    address.clear();
    ticker.clear();
    chain.clear();
    name.clear();
    collected_fees = tvl = total_supply = -1;
    decimal = 0;
}
bool UG_TokenInfo::invalid() const
{
    if (address.length() < 10) return true;
    if (ticker.trimmed().isEmpty() || chain.trimmed().isEmpty()) return true;
    return false;
}
void UG_TokenInfo::toTableRow(QStringList &list) const
{
    list.clear();
    list << address << ticker << name << chain;

    float fv = 1000000;
    list << QString::number(total_supply, 'f', 1);
    list << QString::number(tvl/fv, 'f', 2) << QString::number(collected_fees/float(1000), 'f', 1);
    list << QString::number(decimal);

}
QString UG_TokenInfo::toFileLine() const
{
    QString fline = QString("%1 / %2 / %3").arg(address).arg(ticker).arg(chain);
    fline = QString("%1 / %2").arg(fline).arg(QString::number(tvl, 'f', 1));
    fline = QString("%1 / %2").arg(fline).arg(QString::number(total_supply, 'f', 1));
    fline = QString("%1 / %2").arg(fline).arg(QString::number(collected_fees, 'f', 1));
    fline = QString("%1 / %2 / %3").arg(fline).arg(name).arg(decimal);
    return fline;
}
void UG_TokenInfo::fromFileLine(const QString &fline)
{
    QStringList list = LString::trimSplitList(fline.trimmed(), "/");
    if (list.count() != 8) {qWarning()<<QString("UG_TokenInfo WARNING invalid fline [%1]").arg(fline); return;}

    int i = 0;
    address = list.at(i); i++;
    ticker = list.at(i); i++;
    chain = list.at(i); i++;
    tvl = list.at(i).toDouble(); i++;
    total_supply = list.at(i).toDouble(); i++;
    collected_fees = list.at(i).toDouble(); i++;
    name = list.at(i); i++;
    decimal = list.at(i).toUInt(); i++;
}
