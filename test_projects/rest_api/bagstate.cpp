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
    headers << "N pos" << "Name" << "Ticker" << "Count" << "Price (buy/cur)" << "Margin" << "Profit" << "Type" << "To complete";
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
                sortPositions();
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
void BagState::sortPositions()
{
    int n = m_positions.count();
    if (n < 3) return;
    qDebug()<<QString("BagState::sortPositions() 1    n=%1").arg(n);

    //update finish dates
    for (int i=0; i<n; i++)
        if (m_positions.at(i).isBond())
            emit signalGetBondEndDateByUID(m_positions.at(i).uid, m_positions[i].finish_date);

    //extract stocks
    QList<BagPosition> mid_list;
    for (int i=n-1; i>=0; i--)
        if (!m_positions.at(i).isBond()) mid_list.append(m_positions.takeAt(i));

    qDebug()<<QString("BagState::sortPositions() 2    n=%1/%2").arg(m_positions.count()).arg(mid_list.count());

    //try sort
    n = m_positions.count();
    if (n > 1)
    {
        while (2 > 1)
        {
            bool has_replace = false;
            for (int i=0; i<n-1; i++)
            {
                const QDate &d = m_positions.at(i).finish_date;
                const QDate &d_next = m_positions.at(i+1).finish_date;
                if (d.isValid() && d_next.isValid())
                {
                    if (d_next > d)
                    {
                        BagPosition rec = m_positions.takeAt(i);
                        m_positions.insert(i+1, rec);
                        has_replace = true;
                    }
                }
            }
            if (!has_replace) break;
        }
    }

    //merge containers
    if (!mid_list.isEmpty())
        foreach (const BagPosition &v, mid_list)
            m_positions.prepend(v);


}



