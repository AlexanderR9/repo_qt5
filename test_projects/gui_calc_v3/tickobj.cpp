#include "tickobj.h"

#include <QtMath>
#include <QDebug>



TickObj::TickObj(QObject *parent)
    :LSimpleObject(parent)
{

}
quint16 TickObj::binSize(int fee_type)
{
    switch (fee_type)
    {
        case tst001: return 2;
        case tst005: return 10;
        case tst03: return 60;
        case tst1: return 200;
        default: break;
    }
    return 0;
}
QString TickObj::captionByFeeType(int fee_type)
{
    switch (fee_type)
    {
        case tst001: return "0.01%";
        case tst005: return "0.05%";
        case tst03: return "0.3%";
        case tst1: return "1.0%";
        default: break;
    }
    return "invalid_type";
}
void TickObj::recalc()
{
    m_paramsCalc.reset();
    if (!m_params.validity)
    {
        emit signalError("invalid pool parameters");
        return;
    }

    quint16 t_step = binSize(m_params.fee_type);
    emit signalMsg(QString("fee_size=%1,  tick_step=%2").arg(captionByFeeType(m_params.fee_type)).arg(t_step));
    m_paramsCalc.tick_range.first = tickIndexByPrice(m_params.range.first);
    m_paramsCalc.tick_range.second = tickIndexByPrice(m_params.range.second);
    m_paramsCalc.bin_count = binCount(m_paramsCalc.tick_range.first, m_paramsCalc.tick_range.second);
    m_paramsCalc.fillPrices(t_step);
    m_paramsCalc.findCurrentBin(m_params.cur_price);
    emit signalMsg(QString("range prices %1").arg(m_paramsCalc.range_prices.count()));

    if (m_paramsCalc.cur_bin > 0)
        emit signalMsg(QString("cur_price in %1 bin: [%2; %3]").arg(m_paramsCalc.cur_bin).
                       arg(m_paramsCalc.range_prices.value(m_paramsCalc.tick_range.first+t_step*(m_paramsCalc.cur_bin-1))).
                       arg(m_paramsCalc.range_prices.value(m_paramsCalc.tick_range.first+t_step*m_paramsCalc.cur_bin)));

    //m_paramsCalc.outBinPrices();

    calcTokensSizes();
    m_paramsCalc.assets_sum = m_paramsCalc.token0_size*normalizedCurPrice() + m_paramsCalc.token1_size;

    emit signalSendCalcResult(m_paramsCalc);
}
quint16 TickObj::binCount(int min_tick, int max_tick) const
{
    quint16 n = 0;
    if (!m_params.validity) return n;

    int step = binSize(m_params.fee_type);
    while (2>1)
    {
        n++;
        min_tick += step;
        if (min_tick >= max_tick) break;
    }

    if (min_tick != max_tick)
        qWarning()<<QString("TickObj::binCount WARNING [%1 / %2] number of ticks is not a multiple of %3").arg(min_tick).arg(max_tick).arg(step);

    return n;
}
int TickObj::tickIndexByPrice(double &price)
{
    int t_index = 0;
    quint32 increaser = qPow(2, 15);
    if (price < 1.02 && price > 0.98) increaser = 1;

    int sign = (price > 1) ? 1 : -1;

    double calc_price = -1;
    double prev_price = -1;
    while (increaser >= 1)
    {
        t_index += (sign*increaser);
        calc_price = qPow(basePips(), t_index);
        if (sign > 0 && price > calc_price) {prev_price = calc_price; continue;}
        else if (sign < 0 && price < calc_price) {prev_price = calc_price; continue;}


        if (increaser == 1)
        {
            double d1 = qAbs(price - calc_price);
            double d2 = qAbs(price - prev_price);


            emit signalMsg(QString("LAST t_index=%1  calc_price=%2  prev_price=%3").arg(t_index).
                           arg(QString::number(calc_price, 'f', 6)).arg(QString::number(prev_price, 'f', 6)));

            if (d2 < d1)
            {
                t_index--;
                emit signalMsg("   prev_price nearly");
            }


            break;
        }


        //emit signalMsg(QString("MID: t_index=%1  calc_price=%2  prev_price=%3").arg(t_index).arg(QString::number(calc_price)).arg(QString::number(prev_price)));

        t_index -= (sign*increaser);
        increaser /= 4;
        if (increaser < 10) increaser = 1;

        //emit signalMsg(QString("t_index_roolback=%1  next_increaser=%2").arg(t_index).arg(increaser));
    }

    return t_index;
}
void TickObj::calcTokensSizes()
{
    if (!m_params.validity) return;

    if (m_params.input_token == 0) m_paramsCalc.token0_size = m_params.input_size;
    else m_paramsCalc.token1_size = m_params.input_size;

    double cp = normalizedCurPrice();
    double p1 = m_params.range.first;
    double p2 = m_params.range.second;
    if ((cp <= p1) || (cp >= p2))
    {
        if (m_params.input_token == 0) m_paramsCalc.L = m_paramsCalc.token0_size*(qSqrt(p1)*qSqrt(p2)/(qSqrt(p2) - qSqrt(p1)));
        else m_paramsCalc.L = m_paramsCalc.token1_size/(qSqrt(p2) - qSqrt(p1));
        return;
    }

    //calc for p_min > p_cur > p_max
    double k1 = qSqrt(p2/cp);
    double k2 = qSqrt(p1/cp);
    emit signalMsg(QString("k1=%1   k2=%2").arg(k1).arg(k2));

    if (m_params.input_token == 0)
    {
        double ch = k1*cp*m_paramsCalc.token0_size*(k2-1);
        m_paramsCalc.token1_size = ch/(1-k1);
    }
    else
    {
        double ch = m_paramsCalc.token1_size*(1-k1);
        m_paramsCalc.token0_size = ch/(k1*cp*(k2-1));
    }


    /////////////////calc liquidity////////////////////
    double liq_0 = m_paramsCalc.token0_size*(qSqrt(cp)*qSqrt(p2)/(qSqrt(p2) - qSqrt(cp)));
    double liq_1 = m_paramsCalc.token1_size/(qSqrt(cp) - qSqrt(p1));
    m_paramsCalc.L = qMin(liq_0, liq_1);
    emit signalMsg(QString("liq_0=%1  liq_1=%2").arg(QString::number(liq_0, 'f', 6)).arg(QString::number(liq_1, 'f', 6)));

    //calc changing assets by deltaprice
/*
    double p_next = cp + m_params.delta_price;
    double sqrt_dp = qSqrt(p_next) - qSqrt(cp);
    double sqrt_dp_inverse = 1/qSqrt(p_next) - 1/qSqrt(cp);
    double dx = sqrt_dp_inverse*m_paramsCalc.L;
    double dy = sqrt_dp*m_paramsCalc.L;

    emit signalMsg(QString("AFTER CHANGE PRICE: dx=%1  dy=%2  x_size=%3 y_size=%4  ").arg(QString::number(dx, 'f', 3)).
                                 arg(QString::number(dy, 'f', 3)).arg(QString::number(m_paramsCalc.token0_size+dx, 'f', 3)).
                   arg(QString::number(m_paramsCalc.token1_size+dy, 'f', 3)));
*/
}
void TickObj::slotPriceChanged(float p_next)
{
    double dx = 0;
    double dy = 0;
    double cp = normalizedCurPrice();


    double sqrt_dp = qSqrt(p_next) - qSqrt(cp);
    double sqrt_dp_inverse = 1/qSqrt(p_next) - 1/qSqrt(cp);
    if (qAbs(p_next - m_params.range.first) < 0.001)
    {
        dx = sqrt_dp_inverse*m_paramsCalc.L;
        dy = -1*m_paramsCalc.token1_size;
    }
    else if (qAbs(p_next - m_params.range.second) < 0.001)
    {
        dy = sqrt_dp*m_paramsCalc.L;
        dx = -1*m_paramsCalc.token0_size;
    }
    else
    {
        dx = sqrt_dp_inverse*m_paramsCalc.L;
        dy = sqrt_dp*m_paramsCalc.L;
    }

    emit signalChangePriceResult(dx, dy);
}
double TickObj::normalizedCurPrice() const
{
    double cp = m_params.cur_price;
    if (cp < m_params.range.first) cp = m_params.range.first;
    else if (cp > m_params.range.second) cp = m_params.range.second;
    return cp;
}


//////////////////////////////////////
void PoolParamsStruct::reset()
{
    fee_type = tst005;
    range.first=0; range.second=1;
    cur_price=-1;
    //delta_price = 0;
    input_token=0;
    input_size=1000;
    validity = false;
}
void PoolParamsStruct::setData(const PoolParamsStruct &other)
{
    fee_type = other.fee_type;
    range.first = other.range.first;
    range.second = other.range.second;
    cur_price = other.cur_price;
    //delta_price = other.delta_price;
    input_token = other.input_token;
    input_size =  other.input_size;
    validity =  other.validity;

}
QString PoolParamsStruct::toStr() const
{
    QString s("PoolParams: ");
    s = QString("%1 RANGE[%2; %3]").arg(s).arg(range.first).arg(range.second);
    s = QString("%1 CUR_PRICE[%2]  INPUT_TOKEN[%3 / %4]").arg(s).arg(cur_price).arg(input_token).arg(input_size);    
    //s = QString("%1 DELTA_PRICE[%2]").arg(s).arg(delta_price);
    s = QString("%1 VALIDITY[%2]").arg(s).arg(validity?"Ok":"Err");
    return s;
}
bool PoolParamsStruct::curPriceOutRange() const
{
    if (!validity) return false;
    if (cur_price < range.first || cur_price > range.second) return true;
    return false;
}
//////////////////////////////////////////////////
void PoolParamsCalculated::reset()
{
    token0_size = token1_size = L = 0;
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
      range_prices.insert(i, qPow(TickObj::basePips(), i));
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

    /*
    double price_step = m_params.dRange()/double(m_paramsCalc.bin_count);
    double price = m_params.range.first;
    while (2 > 1)
    {
        m_paramsCalc.cur_bin++;
        price += price_step;
        if (price >= m_params.cur_price)
        {
            double bin_p1 = price - price_step;
            emit signalMsg(QString("cur_price in %1 bin: [%2; %3]").arg(m_paramsCalc.cur_bin).arg(bin_p1).arg(price));
            break;
        }
    }
    */
}


