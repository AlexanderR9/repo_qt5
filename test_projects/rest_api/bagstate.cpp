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
    headers << "N pos" << "Name" << "Ticker" << "Count" << "Price" << "Profit";
    return headers;
}
void BagState::slotLoadPositions(const QJsonObject &j_obj)
{
    qDebug("BagState::slotLoadPositions");
    if (j_obj.isEmpty()) return;
    QStringList keys(j_obj.keys());
    foreach (const QString &v, keys)
    {
        qDebug()<<QString("next key: %1").arg(v);
        if (v == "blocked" || v == "money")
        {
            //qDebug("find blocked");
            const QJsonValue &jv = j_obj.value(v);
            if (jv.isArray())
            {
                const QJsonArray &j_arr = jv.toArray();
                if (!j_arr.isEmpty())
                {
                    qDebug("    is array");
                    if (v == "blocked") m_blocked = InstrumentBase::floatFromJVBlock(j_arr.first());
                    else m_free = InstrumentBase::floatFromJVBlock(j_arr.first());
                }
            }
            //break;
        }
    }

    emit signalBagUpdate();
}
void BagState::slotLoadPortfolio(const QJsonObject &j_obj)
{
    emit signalBagUpdate();

}



