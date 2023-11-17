#include "bagstate.h"
#include "instrument.h"

#include <QJsonObject>
#include <QJsonArray>

// BagState
BagState::BagState(QObject *parent)
    :LSimpleObject(parent)
{
    reset();
}
void BagState::reset()
{
    m_positions.clear();
    m_total = m_blocked = m_free = 0;
}
QStringList BagState::tableHeaders() const
{
    QStringList headers;
    headers << "N pos" << "Name" << "Ticker" << "Count" << "Price (buy/cur)" << "Profit" << "Type" << "To complete";
    return headers;
}
void BagState::slotLoadPositions(const QJsonObject &j_obj)
{
   // qDebug("BagState::slotLoadPositions");
    m_positions.clear();
    if (j_obj.isEmpty()) return;
    QStringList keys(j_obj.keys());
    foreach (const QString &v, keys)
    {
        //qDebug()<<QString("next key: %1").arg(v);
        if (v == "blocked" || v == "money")
        {
            const QJsonValue &jv = j_obj.value(v);
            if (jv.isArray())
            {
                const QJsonArray &j_arr = jv.toArray();
                if (!j_arr.isEmpty())
                {
                    if (v == "blocked") m_blocked = InstrumentBase::floatFromJVBlock(j_arr.first());
                    else m_free = InstrumentBase::floatFromJVBlock(j_arr.first());
                }
            }
        }
    }

    emit signalBagUpdate();
}
void BagState::slotLoadPortfolio(const QJsonObject &j_obj)
{
    qDebug("BagState::slotLoadPortfolio");
    m_positions.clear();
    if (j_obj.isEmpty()) return;

    QStringList keys(j_obj.keys());
    foreach (const QString &v, keys)
    {
        if (v == "totalAmountPortfolio")
        {
            m_total = InstrumentBase::floatFromJVBlock(j_obj.value(v));
        }
        else if (v == "positions")
        {
            if (j_obj.value(v).isArray())
            {
                parsePositions(j_obj.value(v).toArray());
            }
        }
    }
    emit signalBagUpdate();
}
void BagState::parsePositions(const QJsonArray &j_arr)
{
    if (j_arr.isEmpty()) return;

    int i = 0;
    foreach (const QJsonValue &jv, j_arr)
    {
        i++;
        if (jv.isObject())
        {
            const QJsonObject &j_obj = jv.toObject();
            BagPosition pos;
            pos.average_price = InstrumentBase::floatFromJVBlock(j_obj.value("averagePositionPrice"));
            pos.current_price = InstrumentBase::floatFromJVBlock(j_obj.value("currentPrice"));
            pos.count = InstrumentBase::floatFromJVBlock(j_obj.value("quantity"));
            pos.uid = j_obj.value("instrumentUid").toString();
            pos.paper_type = j_obj.value("instrumentType").toString();

            if (pos.invalid()) emit signalError(QString("invalid loading position %1").arg(i));
            else if (pos.paper_type != "currency") m_positions.append(pos);
        }
    }

    if (!m_positions.isEmpty())
        emit signalMsg(QString("Was parsed %1 positions!").arg(m_positions.count()));
}
QString BagState::strPapersCost() const
{
    if (m_positions.isEmpty()) return "---";
    float payed_sum = papersCost_before();
    float cur_sum = papersCost_now();
    return QString("%1 / %2").arg(QString::number(payed_sum, 'f', 1)).arg(QString::number(cur_sum, 'f', 1));
}
QString BagState::strCurProfit() const
{
    if (m_positions.isEmpty()) return "---";
    float profit = papersCost_now() - papersCost_before();
    return QString::number(profit, 'f', 1);
}
float BagState::papersCost_before() const
{
    float sum = 0;
    foreach (const BagPosition &pos, m_positions)
        sum += pos.count*pos.average_price;
    return sum;
}
float BagState::papersCost_now() const
{
    float sum = 0;
    foreach (const BagPosition &pos, m_positions)
        sum += pos.count*pos.current_price;
    return sum;
}



