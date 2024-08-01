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
    QString s("InputPoolParams: ");
    s = QString("%1 RANGE[%2; %3]").arg(s).arg(range.first).arg(range.second);
    s = QString("%1 CUR_PRICE[%2]  INPUT_TOKEN[%3 / %4]").arg(s).arg(cur_price).arg(input_token).arg(input_size);
    //s = QString("%1 DELTA_PRICE[%2]").arg(s).arg(delta_price);
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


//PoolParamsCalculated
void PoolParamsCalculated::reset()
{
    token0_size = token1_size = L = -1;
    tick_range.first=-1;
    tick_range.second=1;
    bin_count = 0;
    cur_bin = -1;
    range_prices.clear();
    assets_sum = 0;
}
void PoolParamsCalculated::fillPrices(int t_step)
{
    range_prices.clear();
    for (int i=tick_range.first; i<=tick_range.second; i+=t_step)
      range_prices.insert(i, qPow(CalcV3Obj::basePips(), i));
}
void PoolParamsCalculated::outBinPrices()
{
    qDebug("BINS PRICES FOR WORKING RANGE");
    QList<int> keys(range_prices.keys());
    for (int i=0; i<keys.count(); i++)
    {
        QString s = QString("%1.  tick=%2   price=%3").arg(i).arg(keys.at(i)).arg(range_prices.value(keys.at(i)));
        if (i == cur_bin) qDebug() << s << "  +";
        qDebug()<<s;
    }
}
void PoolParamsCalculated::findCurrentBin(const double &cp)
{
    if (cp <= 0 || range_prices.count() < 2)
    {
        cur_bin = -1;
        return;
    }

    cur_bin = 0;
    QList<int> keys(range_prices.keys());
    if (cp < range_prices.first() || cp > range_prices.last()) return;

    for (int i=1; i<keys.count(); i++)
    {
        if (range_prices.value(keys.at(i)) > cp)
        {
            cur_bin = i;
            break;
        }
    }
}




