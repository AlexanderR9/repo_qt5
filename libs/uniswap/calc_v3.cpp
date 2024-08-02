#include "calc_v3.h"

#include <QDebug>
#include <QtMath>


// LPoolCalcObj
LPoolCalcObj::LPoolCalcObj(QObject *parent)
	:LSimpleObject(parent),
      m_token0(QString()),
      m_token1(QString()),
      m_pricePrecision(5)
{
    setObjectName("calc_v3_obj");
    reset();
}
void LPoolCalcObj::reset()
{
    m_token0.clear();
    m_token1.clear();
    in_params.reset();
    out_params.reset();
}
void LPoolCalcObj::setPoolTokens(QString t0, QString t1, int fee_type)
{
    m_token0 = t0.trimmed();
    m_token1 = t1.trimmed();
    in_params.fee_type = fee_type;
    updateInputParamsValidity();

    emit signalMsg(QString("LPoolCalcObj::setPoolTokens   token0[%1]  token1[%2]  fee[%3]").arg(m_token0).arg(m_token1).arg(captionByFeeType(in_params.fee_type)));
}
void LPoolCalcObj::setPricesRange(double p1, double p2)
{
    in_params.range.first = p1;
    in_params.range.second = p2;
    updateInputParamsValidity();
}
void LPoolCalcObj::setCurPrice(double cp)
{
    in_params.cur_price = cp;
    updateInputParamsValidity();
}
void LPoolCalcObj::recalc()
{
    emit signalMsg("LPoolCalcObj:  try recalc ....");
    out_params.reset();
    updateInputParamsValidity();
    if (invalidState())
    {
        emit signalError(QString("LPoolCalcObj: invalid input params"));
        return;
    }

    nomalizeRangeByBinSize();
    calcLiquiditySize();
    calcTokenSizes();
}
bool LPoolCalcObj::invalidState() const
{
    if (m_token0.isEmpty() || m_token1.isEmpty()) return true;
    if (!in_params.validity) return true;
    return false;
}
void LPoolCalcObj::updateInputParamsValidity()
{
    in_params.validity = false;
    if (invalidFeeType(in_params.fee_type)) return;
    if (in_params.range.first >= in_params.range.second) return;
    if (in_params.range.first < 0) return;
    if (in_params.input_token > 1) return;
    if (in_params.input_size < 0.001) return;

    in_params.normalizeCurPrice();
    in_params.validity = true;
}
void LPoolCalcObj::setTokenSize(double t_size, quint8 t_index)
{
    in_params.input_size = t_size;
    in_params.input_token = ((t_index>1) ? 1 : 0);
}
void LPoolCalcObj::calcLiquiditySize()
{
    if (invalidState()) return;

    double p1 = in_params.range.first;
    double p2 = in_params.range.second;
    double t_size = in_params.input_size;
    double cp = in_params.cur_price;
    if (cp <= p1)
    {
        out_params.L = (t_size*qSqrt(p1*p2))/(qSqrt(p2) - qSqrt(p1));
    }
    else if (cp >= p2)
    {
        out_params.L = t_size/(qSqrt(p2) - qSqrt(p1));
    }
    else // cur price is remaining into range [p1, p2]
    {
        if (in_params.input_token == 1) out_params.L = t_size/(qSqrt(cp) - qSqrt(p1));
        else out_params.L = (t_size*qSqrt(cp*p2))/(qSqrt(p2) - qSqrt(cp));
    }
}
void LPoolCalcObj::nomalizeRangeByBinSize()
{
    if (invalidState()) return;

    out_params.tick_range.first = tickIndexByPrice(in_params.range.first);
    in_params.range.first = tickPrice(out_params.tick_range.first);
    out_params.tick_range.second = tickIndexByPrice(in_params.range.second);
    in_params.range.second = tickPrice(out_params.tick_range.second);

    in_params.normalizeCurPrice();
    out_params.fillPrices(binSize(in_params.fee_type));
}
void LPoolCalcObj::calcTokenSizes()
{
    if (in_params.input_token == 0) out_params.token0_size = in_params.input_size;
    else out_params.token1_size = in_params.input_size;
    if (in_params.curPriceIsBound()) return;

    double L2 = qPow(out_params.L, 2);
    if (in_params.input_token == 0) out_params.token1_size = L2/out_params.token0_size;
    else out_params.token0_size = L2/out_params.token1_size;
}
void LPoolCalcObj::getGuiParams(GuiPoolResults &p)
{
    p.reset();
    if (invalidState()) return;

    p.cur_price = in_params.cur_price;
    p.L = out_params.L;
    p.price_range = in_params.range;
    p.tick_range = out_params.tick_range;
    p.token_sizes.first = out_params.token0_size;
    p.token_sizes.second = out_params.token1_size;
}
double LPoolCalcObj::assetsSum(const double &cp) const
{
    if (invalidState()) return -1;
    return out_params.assetsSum(cp);
}



// static funcs
quint16 LPoolCalcObj::binSize(int fee_type)
{
    switch (fee_type)
    {
        case pfs001: return 2;
        case pfs005: return 10;
        case pfs025: return 50;
        case pfs03: return 60;
        case pfs1: return 200;
        default: break;
    }
    return 0;
}
QString LPoolCalcObj::captionByFeeType(int fee_type)
{
    switch (fee_type)
    {
        case pfs001: return "0.01%";
        case pfs005: return "0.05%";
        case pfs025: return "0.25%";
        case pfs03: return "0.3%";
        case pfs1: return "1.0%";
        default: break;
    }
    return "invalid_type";
}
bool LPoolCalcObj::invalidFeeType(int ft)
{
    if (ft == pfs001) return false;
    if (ft == pfs005) return false;
    if (ft == pfs025) return false;
    if (ft == pfs03) return false;
    if (ft == pfs1) return false;
    return true;
}
qint64 LPoolCalcObj::tickIndexByPrice(double &price)
{
    qint64 i_tick = 0;
    if (price == 1) return i_tick;

    quint32 increaser = qPow(2, 15);
    if (price > 0.98 && price < 1.02) increaser = 1;
    int sign = (price > 1) ? 1 : -1;

    while (increaser >= 1)
    {
        i_tick += (sign*increaser);
        double tick_price = tickPrice(i_tick);
        if (sign > 0 && price > tick_price) continue;
        if (sign < 0 && price < tick_price) continue;

        if (increaser > 1)
        {
            i_tick -= (sign*increaser);
            increaser /= 4;
            if (increaser < 10) increaser = 1;
            continue;
        }
        break;
    }
    return i_tick;
}
double LPoolCalcObj::tickPrice(qint64 i_tick)
{
    if (i_tick == 0) return 1;
    return qPow(basePips(), i_tick);
}
