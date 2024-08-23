#include "poolstructs.h"
#include "calc_v3.h"

#include <QtMath>
#include <QDebug>

//InputPoolParams
void InputPoolParams::reset()
{
    fee_type = pfs005;
    range.first = 0; range.second = 1;
    cur_price = -1;
    input_token = 0;
    input_size = 1000;
    validity = false;
}
void InputPoolParams::setData(const InputPoolParams &other)
{
    fee_type = other.fee_type;
    range.first = other.range.first;
    range.second = other.range.second;
    cur_price = other.cur_price;
    input_token = other.input_token;
    input_size =  other.input_size;
    validity =  other.validity;
}
QString InputPoolParams::toStr() const
{
    QString s = QString("InputPoolParams: FEE(%1)").arg(LPoolCalcObj::captionByFeeType(fee_type));
    s = QString("%1 PRICE_RANGE[%2; %3]").arg(s).arg(range.first).arg(range.second);
    s = QString("%1 CUR_PRICE[%2]  INPUT_TOKEN[%3 / %4]").arg(s).arg(cur_price).arg(input_token).arg(input_size);
    s = QString("%1 VALIDITY[%2]").arg(s).arg(validity?"Ok":"Err");
    return s;
}
bool InputPoolParams::curPriceOutRange() const
{
    if (!validity) return false;
    if (cur_price < range.first || cur_price > range.second) return true;
    return false;
}
void InputPoolParams::normalizeCurPrice()
{
    if (range.first <= 0 || range.first >= range.second) return;
    if (cur_price > range.first && cur_price < range.second) return;
    if (cur_price < 0) return;

    if (cur_price < range.first) {cur_price = range.first; input_token = 0;}
    if (cur_price > range.second) {cur_price = range.second; input_token = 1;}
}
bool InputPoolParams::curPriceIsBound() const
{
    if (!validity) return false;
    if (cur_price <= range.first) return true;
    if (cur_price >= range.second) return true;
    return false;
}

//PoolParamsCalculated
void PoolParamsCalculated::reset()
{
    token0_size = token1_size = L = 0;
    tick_range.first = tick_range.second = 0;
    bin_prices.clear();
}
double PoolParamsCalculated::assetsSum(const double &cp) const
{
    return (token0_size*cp + token1_size);
}
void PoolParamsCalculated::fillPrices(int fee_type)
{
    bin_prices.clear();
    int t_step = LPoolCalcObj::binSize(fee_type);
    if (t_step <= 0) return;

    for (int i=tick_range.first; i<=tick_range.second; i+=t_step)
      bin_prices.insert(i, LPoolCalcObj::tickPrice(i));
}
QString PoolParamsCalculated::toStr() const
{
    QString s = QString("PoolParamsCalculated: tick_range(%1/%2)").arg(tick_range.first).arg(tick_range.second);
    s = QString("%1 BINS[%2]").arg(s).arg(bin_prices.size());
    s = QString("%1 TOKEN_SIZES[%2 / %3]").arg(s).arg(token0_size).arg(token1_size);
    s = QString("%1 L[%2]").arg(s).arg(QString::number(L, 'f', 3));
    return s;
}




//GuiPoolResults
void GuiPoolResults::reset()
{
    cur_price = L = 0;
    token_sizes.first = token_sizes.second = 0;
    price_range.first = price_range.second = 0;
    tick_range.first = tick_range.second = 0;
}
quint32 GuiPoolResults::binCount(int fee_type) const
{
    int t_step = LPoolCalcObj::binSize(fee_type);
    if (t_step <= 0) return -1;
    return qAbs(tick_range.second - tick_range.first)/t_step;
}

