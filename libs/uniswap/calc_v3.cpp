#include "calc_v3.h"

#include "lstring.h"
#include "lstatic.h"
#include "lfile.h"
  
  
#include <QDebug>
#include <QColor>
#include <QDir>
#include <QtMath>
#include <QIODevice>
#include <QByteArray>

#define MSG_PRIOR	1


// CalcV3Obj
CalcV3Obj::CalcV3Obj(QObject *parent)
	:LSimpleObject(parent),
      m_token0(QString()),
      m_token1(QString())
{
    reset();
}
void CalcV3Obj::reset()
{
    m_token0.clear();
    m_token1.clear();
    in_params.reset();
    out_params.reset();
}
void CalcV3Obj::setPoolTokens(QString t0, QString t1, int fee_type)
{
    m_token0 = t0.trimmed();
    m_token1 = t1.trimmed();
    in_params.fee_type = fee_type;
    updateInputParamsValidity();
}
void CalcV3Obj::setPricesRange(double p1, double p2)
{
    in_params.range.first = p1;
    in_params.range.second = p2;
    updateInputParamsValidity();
}
void CalcV3Obj::setCurPrice(double cp)
{
    in_params.cur_price = cp;
    updateInputParamsValidity();
}
void CalcV3Obj::calc()
{
    out_params.reset();
    updateInputParamsValidity();
    if (invalidState())
    {
        emit signalError(QString("CalcV3Obj: invalid input params"));
        return;
    }



}
bool CalcV3Obj::invalidState() const
{
    if (m_token0.isEmpty() || m_token1.isEmpty()) return true;
    if (!in_params.validity) return true;
    return false;
}
void CalcV3Obj::updateInputParamsValidity()
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
void CalcV3Obj::setTokenSize(double t_size, quint8 t_index)
{
    in_params.input_size = t_size;
    in_params.input_token = ((t_index>1) ? 1 : 0);

}
void CalcV3Obj::calcLiquiditySize()
{
    if (invalidState()) return;
}
void CalcV3Obj::nomalizeRangeByBinSize()
{
    if (invalidState()) return;


}



// static funcs
quint16 CalcV3Obj::binSize(int fee_type)
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
QString CalcV3Obj::captionByFeeType(int fee_type)
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
bool CalcV3Obj::invalidFeeType(int ft)
{
    if (ft == pfs001) return false;
    if (ft == pfs005) return false;
    if (ft == pfs025) return false;
    if (ft == pfs03) return false;
    if (ft == pfs1) return false;
    return true;
}
qint64 CalcV3Obj::tickIndexByPrice(double &price)
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
double CalcV3Obj::tickPrice(qint64 i_tick)
{
    if (i_tick == 0) return 1;
    return qPow(basePips(), i_tick);
}
