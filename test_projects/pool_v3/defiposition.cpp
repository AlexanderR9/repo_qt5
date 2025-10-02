#include "defiposition.h"
#include "lstring.h"
#include "lmath.h"
#include "deficonfig.h"
#include "appcommonsettings.h"

#include <QDebug>
#include <QJsonObject>



// DefiPosition
void DefiPosition::reset()
{
    token_addrs.first.clear();
    token_addrs.second.clear();
    pid = -1;
    fee = 0;
    tick_range.first = tick_range.second = 0;
    liq.clear();

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
    if (invalid()) return QString("[? : ?]");
    QString p1 = QString::number(state.price0_range.first, 'f', AppCommonSettings::interfacePricePrecision(state.price0_range.first));
    QString p2 = QString::number(state.price0_range.second, 'f', AppCommonSettings::interfacePricePrecision(state.price0_range.second));
    return QString("[%1 : %2]").arg(p1).arg(p2);
}
QString DefiPosition::interfaceCurrentPrice() const
{
    if (invalid()) return QString("-1");
    return QString::number(state.price0, 'f', AppCommonSettings::interfacePricePrecision(state.price0));
}
QString DefiPosition::interfaceAssetAmounts() const
{
    if (invalid()) return QString("? / ?");

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


