#include "defiposition.h"
#include "lstring.h"
#include "lmath.h"
#include "deficonfig.h"
#include "appcommonsettings.h"

#include <QDebug>
#include <QJsonObject>


#define STABLE_POOL_PRICE_PRECISION     6


// DefiPosition
void DefiPosition::reset()
{
    token_addrs.first.clear();
    token_addrs.second.clear();
    token_names.first = token_names.second = "?";

    pid = -1;
    fee = 0;
    tick_range.first = tick_range.second = 0;
    liq.clear();
    tprice_index_desired = tamount_index_desired = 0;
    stable_pool = false;

    state.reset();
}
bool DefiPosition::invalid() const
{
    if (pid <= 0) return true;
    if (token_addrs.first.length() < 20 || token_addrs.second.length() < 20) return true;
    if (tick_range.first >= tick_range.second) return true;
    return false;
}
bool DefiPosition::hasLiquidity() const
{
    if (liq.trimmed().isEmpty() || liq == QString("0")) return false;
    return true;
}
void DefiPosition::fromFileLine(const QString &fline)
{
    reset();
    if (fline.trimmed().isEmpty()) return;
    if (fline.contains("#")) return;

    QStringList list = LString::trimSplitList(fline, "/");
    if (list.count() != 7)
    {
        qWarning()<<QString("DefiPosition: WARNING invalid fline fields count(%1) != 7").arg(list.count());
        return;
    }

    bool ok;
    int i = 0;
    pid = list.at(i).trimmed().toInt(&ok); i++;
    if (!ok) {reset(); return;}
    liq = list.at(i).trimmed(); i++;
    fee = list.at(i).trimmed().toInt(&ok); i++;
    if (!ok) {reset(); return;}
    tick_range.first = list.at(i).trimmed().toInt(&ok); i++;
    if (!ok) {reset(); return;}
    tick_range.second = list.at(i).trimmed().toInt(&ok); i++;
    if (!ok) {reset(); return;}
    token_addrs.first = list.at(i).trimmed(); i++;
    token_addrs.second = list.at(i).trimmed(); i++;

}
void DefiPosition::calcTIndex(QString chain_name)
{
    tprice_index_desired = tamount_index_desired = 0;
    if (invalid()) return;

    int cid = defi_config.getChainID(chain_name);
    if (cid < 0) return;

    int pos0 = defi_config.getTokenIndex(token_addrs.first, cid);
    int pos1 = defi_config.getTokenIndex(token_addrs.second, cid);
    if (pos0 >= 0 && pos1 >= 0) // определяем является ли пул is_stable
        stable_pool = (defi_config.tokens.at(pos0).is_stable && defi_config.tokens.at(pos1).is_stable);

    token_names.first = defi_config.tokenNameByAddress(token_addrs.first, cid);
    token_names.second = defi_config.tokenNameByAddress(token_addrs.second, cid);
    QString pair = QString("%1/%2").arg(token_names.first).arg(token_names.second);
    if (pair.contains("?")) return;

    tprice_index_desired = defi_config.getPoolTokenPriceIndex(pair);
    tamount_index_desired = defi_config.getPoolTokenAmountIndex(pair);

}
QString DefiPosition::toStr() const
{
    QString s("DefiPosition:");
    s = QString("%1 PID[%2] tokens[%3 / %4]").arg(s).arg(pid).arg(token_addrs.first).arg(token_addrs.second);
    s = QString("%1 fee[%2] tick_range[%3 / %4]").arg(s).arg(fee).arg(tick_range.first).arg(tick_range.second);
    s = QString("%1 liquidity[%2]").arg(s).arg(liq);
    return s;
}
QString DefiPosition::interfaceFee() const
{
    if (invalid()) return "---";
    float a = float(fee)/float(10000);
    return QString("%1%").arg(QString::number(a, 'f', 2));
}
QString DefiPosition::interfaceTickRange() const
{
    if (invalid()) return QString("[? : ?]");
    return QString("[%1 : %2]").arg(tick_range.first).arg(tick_range.second);
}
QString DefiPosition::interfacePriceRange() const
{
    if (invalid() || state.invalid()) return QString("[? : ?]");

    float p_low = state.price0_range.first;
    float p_high = state.price0_range.second;
    if (tprice_index_desired == 1)
    {
        float x = float(1)/p_high;
        p_high = float(1)/p_low;
        p_low = x;
    }

    quint8 prec = (stable_pool ? STABLE_POOL_PRICE_PRECISION : AppCommonSettings::interfacePricePrecision(p_low));
    QString p1 = QString::number(p_low, 'f', prec);
    QString p2 = QString::number(p_high, 'f', prec);
    return QString("[%1 : %2]").arg(p1).arg(p2);
}
QString DefiPosition::interfaceCurrentPrice() const
{
    if (invalid() || state.invalid()) return QString("-1");

    float p = ((tprice_index_desired == 1) ? state.price1() : state.price0);
    quint8 prec = (stable_pool ? STABLE_POOL_PRICE_PRECISION : AppCommonSettings::interfacePricePrecision(p));
    return QString::number(p, 'f', prec);
}
QString DefiPosition::interfaceAssetAmounts() const
{
    if (invalid()) return QString("? / ?");
    if (!hasLiquidity()) return QString("- / -");

    QString size0 = QString::number(state.asset_amounts.first, 'f', AppCommonSettings::interfacePricePrecision(state.asset_amounts.first));
    QString size1 = QString::number(state.asset_amounts.second, 'f', AppCommonSettings::interfacePricePrecision(state.asset_amounts.second));
    return QString("%1 / %2").arg(size0).arg(size1);
}
QString DefiPosition::interfaceRewards() const
{
    if (invalid()) return QString("? / ?");

    QString size0 = QString::number(state.rewards.first, 'f', AppCommonSettings::interfacePricePrecision(state.rewards.first));
    QString size1 = QString::number(state.rewards.second, 'f', AppCommonSettings::interfacePricePrecision(state.rewards.second));
    return QString("%1 / %2").arg(size0).arg(size1);
}
QString DefiPosition::interfaceAssetAmountsDesired() const
{
    float a = lockedDesiredSum();
    if (a < 0) return QString("-1.0 ???"); // invalid state

    QString t_name = ((tamount_index_desired == 1) ? token_names.second : token_names.first);
    return QString("%1 %2").arg(QString::number(a, 'f', AppCommonSettings::interfacePricePrecision(a))).arg(t_name);
}
QString DefiPosition::interfaceRewardsDesired() const
{
    float a = rewardDesiredSum();
    if (a < 0) return QString("-1.0 ???"); // invalid state

    QString t_name = ((tamount_index_desired == 1) ? token_names.second : token_names.first);
    return QString("%1 %2").arg(QString::number(a, 'f', AppCommonSettings::interfacePricePrecision(a))).arg(t_name);
}
float DefiPosition::lockedDesiredSum() const
{
    if (state.invalid()) return -1;
    if (!hasLiquidity()) return 0;
    float a0 = state.asset_amounts.first;
    float a1 = state.asset_amounts.second;
    if (tamount_index_desired == 0) return (a0 + (a1/state.price0));
    if (tamount_index_desired == 1) return (a1 + (a0*state.price0));
    return 0;
}
float DefiPosition::lockedSum() const
{
    if (state.invalid()) return -1;
    if (!hasLiquidity()) return 0;
    float p0 = defi_config.lastPriceByTokenName(token_names.first);
    float p1 = defi_config.lastPriceByTokenName(token_names.second);
    if (p0 < 0 || p1 < 0) return -1;
    return (state.asset_amounts.first*p0 + state.asset_amounts.second*p1);
}
float DefiPosition::rewardDesiredSum() const
{
    if (state.invalid()) return -1;
    //if (!hasLiquidity()) return 0;
    float a0 = state.rewards.first;
    float a1 = state.rewards.second;
    if (tamount_index_desired == 0) return (a0 + (a1/state.price0));
    if (tamount_index_desired == 1) return (a1 + (a0*state.price0));
    return 0;
}
float DefiPosition::rewardSum() const
{
    if (state.invalid()) return -1;
    //if (!hasLiquidity()) return 0;
    float p0 = defi_config.lastPriceByTokenName(token_names.first);
    float p1 = defi_config.lastPriceByTokenName(token_names.second);
    if (p0 < 0 || p1 < 0) return -1;
    return (state.rewards.first*p0 + state.rewards.second*p1);
}
bool DefiPosition::isOutRange() const
{
    if (state.invalid() || !hasLiquidity()) return false;
    return ((state.pool_tick < tick_range.first) || (state.pool_tick >= tick_range.second));
}


////////////////DefiPositionState//////////////////
void DefiPositionState::reset()
{
    price0 = -1;
    pool_tick = 0;
    price0_range.first = price0_range.second = 0;
    asset_amounts.first = asset_amounts.second = 0;
    rewards.first = rewards.second = 0;

}
void DefiPositionState::update(const QJsonObject &j_obj)
{
    reset();

    bool ok;
    float p = j_obj.value("price0").toString().toFloat(&ok);
    if (!ok || p<=0) qWarning()<<QString("DefiPositionState: WARNING - invalid price0 field");
    else price0 = p;

    int t = j_obj.value("tick").toString().toInt(&ok);
    if (!ok) qWarning()<<QString("DefiPositionState: WARNING - invalid tick field");
    else pool_tick = t;

    //parse price range
    p = j_obj.value("range_p1").toString().toFloat(&ok);
    if (!ok || p<=0) qWarning()<<QString("DefiPositionState: WARNING - invalid range_p1 field");
    else price0_range.first = p;
    p = j_obj.value("range_p2").toString().toFloat(&ok);
    if (!ok || p<=0) qWarning()<<QString("DefiPositionState: WARNING - invalid range_p2 field");
    else price0_range.second = p;

    //parse assets amount
    p = j_obj.value("asset0").toString().toFloat(&ok);
    if (!ok || p<0) qWarning()<<QString("DefiPositionState: WARNING - invalid asset0 field");
    else asset_amounts.first = p;
    p = j_obj.value("asset1").toString().toFloat(&ok);
    if (!ok || p<0) qWarning()<<QString("DefiPositionState: WARNING - invalid asset1 field");
    else asset_amounts.second = p;

    //parse unclaimed rewards
    p = j_obj.value("reward0").toString().toFloat(&ok);
    if (!ok || p<0) qWarning()<<QString("DefiPositionState: WARNING - invalid reward0 field");
    else rewards.first = p;
    p = j_obj.value("reward1").toString().toFloat(&ok);
    if (!ok || p<0) qWarning()<<QString("DefiPositionState: WARNING - invalid reward1 field");
    else rewards.second = p;
}
bool DefiPositionState::invalid() const
{
    if (price0 <= 0) return true;
    if (price0_range.first <= 0 || price0_range.second <= 0) return true;
    return false;
}
float DefiPositionState::price1() const
{
    if (invalid()) return -1;
    if (price0 <= 0) return 0;
    return float(1)/price0;
}

