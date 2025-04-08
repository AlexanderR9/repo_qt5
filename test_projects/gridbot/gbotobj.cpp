#include "gbotobj.h"

#include <QDebug>


/////////////// GBotObj ///////////////////
GBotObj::GBotObj(QObject *parent)
    :LSimpleObject(parent)
{


}
void GBotObj::setParams(const GBotParams &p)
{
    m_params.in_sum = p.in_sum;
    m_params.leverage = p.leverage;
    m_params.grid_count = p.grid_count;
    m_params.range = p.range;
    m_params.start_price = p.start_price;
    m_params.direction = p.direction;
}
void GBotObj::resetParams()
{
    GBotParams p;
    setParams(p);
}
void GBotObj::exec()
{
    m_calcData.reset();
    if (m_params.invalid())
    {
        emit signalError("parameters is invalid");
        return;
    }

    qDebug("GBotObj::exec()");

    m_calcData.grid_step = (m_params.range.second - m_params.range.first)/m_params.grid_count; //шаг цены
    m_calcData.lot_step = m_params.in_sum*m_params.leverage/(m_params.start_price*m_params.grid_count); // объем актива на каждом шаге
    calcGridValues();

    switch (m_params.direction)
    {
        case GBotParams::tdLong: {m_calcData.lot_start = m_calcData.lot_step*upSteps(); break;}
        case GBotParams::tdShort: {m_calcData.lot_start = m_calcData.lot_step*downSteps(); break;}
        default: break;
    }
    QString msg("CALC_PARAMS:");
    msg = QString("%1  start_amount=%2  grid_step=%3").arg(msg).arg(QString::number(m_calcData.lot_start, 'f', 2))
            .arg(QString::number(m_calcData.grid_step, 'f', 4));
    emit signalMsg(msg);

    QString step_profit = QString("step_profit=%1(%2%)").arg(QString::number(stepProfit(false), 'f', 2))
            .arg(QString::number(stepProfit(true), 'f', 2));
    emit signalMsg(step_profit);

    calcTableLines();


   // qDebug()<<QString("p_step=%1 lot_step=%2 steps down/up  %3/%4").arg(grid_step).arg(lot_step).arg(steps_down).arg(steps_up);
   // qDebug()<<QString;
}
float GBotObj::stepProfit(bool is_persent) const
{
    float profit = m_calcData.grid_step*m_calcData.lot_step;
    if (is_persent) return (100*profit/m_params.in_sum);
    return profit;
}
void GBotObj::calcGridValues()
{
    int steps_down = 0; // шагов ниже start_price
    int steps_up = 0; // шагов выше start_price

    float a = m_params.range.first;
    while (2 > 1)
    {
        bool need_add = false;
        float d = a - m_params.start_price;
        if (a < m_params.start_price)
        {
            if (qAbs(d) < m_calcData.grid_step && isShort()) {}
            else {steps_down++; need_add = true;}
        }
        if (a > m_params.start_price)
        {
            if (qAbs(d) < m_calcData.grid_step && isLong()) {}
            else {steps_up++; need_add = true;}
        }
        if (need_add) m_calcData.grid_values.append(a);

        a += m_calcData.grid_step;
        if (a > m_params.range.second) break;
    }
}
void GBotObj::calcTableLines()
{
    QStringList table_data;

    int up_steps = upSteps();
    int down_steps = downSteps();

    // grid_value / step_number / step_lot / trade_type / amount_lot / step_profit / step_result
    QString start_line = QString("start / 0 / 0 / --- / --- / %1 / 0 / %2 / 0").arg(m_calcData.lot_start).arg(m_params.in_sum);
    table_data.append(start_line);

    ////////BELOW STEPS/////////////
    float q = m_calcData.lot_start;
    float res = m_params.in_sum;
    for (int i=0; i<down_steps; i++) // смотрим шаги ниже точки входа
    {
        float g_value = m_calcData.grid_values.at(down_steps-i-1); // текущее значение сетки
        float d_price = qAbs(g_value - m_params.start_price); //текущая дельта на этом шаге
        float deviation_p = (-1*d_price*100)*m_params.start_price; //отклонение цены на этом шаге,%

        float step_profit = 0;
        if (isLong()) step_profit = -1*m_calcData.grid_step*q; // насколько еще просели
        else step_profit = d_price*m_calcData.lot_step; //win step
        res += step_profit;

        if (isLong()) q += m_calcData.lot_step; //идет накопление
        else q -= m_calcData.lot_step;

        QString line = QString::number(g_value, 'f', 3);
        line = QString("%1 / %2").arg(line).arg(QString::number(deviation_p, 'f', 2));
        line = QString("%1 / %2 / %3").arg(line).arg(i+1).arg(QString::number(m_calcData.lot_step, 'f', 2));
        line = QString("%1 / %2").arg(line).arg(isLong() ? "buy" : "sell");
        line = QString("%1 / %2").arg(line).arg(QString::number(q, 'f', 2));
        line = QString("%1 / %2").arg(line).arg(QString::number(step_profit, 'f', 2));
        line = QString("%1 / %2").arg(line).arg(QString::number(res, 'f', 2));
        deviation_p = 100.0*(res-m_params.in_sum)/m_params.in_sum;
        line = QString("%1 / %2(%3%)").arg(line).arg(QString::number(res-m_params.in_sum, 'f', 2)).arg(QString::number(deviation_p, 'f', 1));
        table_data.append(line);
    }

    ////////ABOVE STEPS/////////////
    q = m_calcData.lot_start;
    res = m_params.in_sum;
    for (int i=0; i<up_steps; i++) // смотрим шаги выше точки входа
    {
        float g_value = m_calcData.grid_values.at(m_calcData.grid_values.count()-up_steps+i); // текущее значение сетки
        float d_price = qAbs(g_value - m_params.start_price); //текущая дельта на этом шаге
        float deviation_p = (d_price*100)*m_params.start_price; //отклонение цены на этом шаге,%

        float step_profit = 0;
        if (isShort()) step_profit = -1*m_calcData.grid_step*q; // насколько еще просели
        else step_profit = d_price*m_calcData.lot_step;
        res += step_profit;

        if (isShort()) q += m_calcData.lot_step; //идет накопление
        else q -= m_calcData.lot_step;

        QString line = QString::number(g_value, 'f', 3);
        line = QString("%1 / %2").arg(line).arg(QString::number(deviation_p, 'f', 2));
        line = QString("%1 / %2 / %3").arg(line).arg(i+1).arg(QString::number(m_calcData.lot_step, 'f', 2));
        line = QString("%1 / %2").arg(line).arg(isShort() ? "buy" : "sell");
        line = QString("%1 / %2").arg(line).arg(QString::number(q, 'f', 2));
        line = QString("%1 / %2").arg(line).arg(QString::number(step_profit, 'f', 2));
        line = QString("%1 / %2").arg(line).arg(QString::number(res, 'f', 2));
        deviation_p = 100.0*(res-m_params.in_sum)/m_params.in_sum;
        line = QString("%1 / %2(%3%)").arg(line).arg(QString::number(res-m_params.in_sum, 'f', 2)).arg(QString::number(deviation_p, 'f', 1));
        table_data.prepend(line);
    }

    emit signalTableResult(table_data);
}
int GBotObj::upSteps() const
{
    int n = 0;
    foreach (float a, m_calcData.grid_values)
        if (a > m_params.start_price) n++;
    return n;
}
int GBotObj::downSteps() const
{
    int n = 0;
    foreach (float a, m_calcData.grid_values)
        if (a < m_params.start_price) n++;
    return n;
}

