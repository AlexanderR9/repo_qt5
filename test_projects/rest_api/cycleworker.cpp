#include "cycleworker.h"
#include "lstring.h"
#include "apicommonsettings.h"

#include <QTimer>
#include <QDebug>


#define PRICE_BLOCK_SIZE    20


// CycleWorker
CycleWorker::CycleWorker(QObject *parent)
    :LSimpleObject(parent),
      m_timer(NULL),
      m_mode(cmNone)
{
    m_timer = new QTimer(this);
    m_timer->setInterval(api_commonSettings.i_history.timeout);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));

}
void CycleWorker::slotTimer()
{
    qDebug()<<QString("CycleWorker::slotTimer() m_cycleData size %1").arg(m_cycleData.count());
    if (m_cycleData.isEmpty())
    {
        reset();
        m_timer->stop();
        emit signalFinished();
        return;
    }

    emit signalNextReq();
}
void CycleWorker::breakCycle()
{
    reset();
    m_timer->stop();
}
void CycleWorker::reset()
{
    m_mode = cmNone;
    m_cycleData.clear();
    m_apiMetod.clear();
}
void CycleWorker::checkCycleMode(const QString &src)
{
    reset();
    if (!src.contains("CycleMode")) return;

    int pos = src.indexOf('/');
    if (pos < 0)
    {
        emit signalError(QString("CycleWorker: invalid SRC - %1").arg(src));
        return;
    }
    QString cycle_name = LString::strTrimLeft(src, pos+1);
    m_apiMetod = api_commonSettings.cycle_metods.value(cycle_name, QString());
    if (m_apiMetod.isEmpty())
    {
        emit signalError(QString("CycleWorker: Not found API_metod for cycle metod [%1]").arg(cycle_name));
        return;
    }

    if (src.toLower().contains("coupon")) m_mode = cmCoupons;
    else if (src.toLower().contains("div")) m_mode = cmDivs;
    else if (src.toLower().contains("history")) m_mode = cmHistory;
    else if (src.toLower().contains("price")) m_mode = cmPrices;
}
void CycleWorker::prepareCycleData(QStringList &i_list)
{
    if (i_list.isEmpty())
    {
        emit signalError(QString("CycleWorker: cycle_data is empty, CYCLE MODE STOPED"));
        return;
    }

    foreach (const QString &v, i_list)
    {
        QStringList list = LString::trimSplitList(v, " : ");
        if (list.count() != 3)
        {
            qWarning()<<QString("CycleWorker::prepareCycleData() WARNING line instrument invalid: %1").arg(v);
            continue;
        }
        m_cycleData.append(CycleItem(list.at(0), list.at(1), list.at(2)));
    }
    trimDataByAPIMetod();

    if (m_cycleData.isEmpty())
    {
        emit signalError(QString("CycleWorker: cycle_data is empty, CYCLE MODE STOPED"));
        return;
    }

    emit signalMsg(QString("STARTING CYCLE MODE: api_metod[%1]  cycle_data size %2").arg(m_apiMetod).arg(m_cycleData.count()));
    m_timer->start();
}
void CycleWorker::trimDataByAPIMetod()
{
    if (m_cycleData.isEmpty()) return;

    QString sign;
    switch (m_mode)
    {
        case cmCoupons: {sign = "bond"; break;}
        case cmDivs: {sign = "stock"; break;}
        default: break;
    }
    if (sign.isEmpty()) return;

    int n = m_cycleData.count();
    for (int i=n-1; i>=0; i--)
        if (m_cycleData.at(i).type != sign) m_cycleData.removeAt(i);
}
void CycleWorker::getNextCycleData(QStringList &list)
{
    list.clear();
    if (m_cycleData.isEmpty()) return;

    switch (m_mode)
    {
        case cmCoupons:
        case cmDivs:
        {
            list << m_cycleData.first().figi;
            m_cycleData.removeFirst();
            break;
        }
        case cmHistory:
        {
            list << m_cycleData.first().uid;
            m_cycleData.removeFirst();
            break;
        }
        case cmPrices:
        {
            for (int i=0; i<PRICE_BLOCK_SIZE; i++)
            {
                list << m_cycleData.first().uid;
                m_cycleData.removeFirst();
                if (m_cycleData.isEmpty()) break;
            }
            break;
        }
        default: break;
    }
}

