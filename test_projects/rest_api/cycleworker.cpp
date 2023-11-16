#include "cycleworker.h"
#include "lstring.h"
#include "apicommonsettings.h"
#include "instrument.h"
#include "lstaticxml.h"
#include "lfile.h"

#include <QTimer>
#include <QDebug>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include <QDir>
#include <QDomDocument>


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

    QString s = src.toLower().trimmed();
    if (s.contains("coupon")) m_mode = cmCoupons;
    else if (s.contains("div")) m_mode = cmDivs;
    else if (s.contains("history")) m_mode = cmHistory;
    else if (s.contains("price") && s.contains("bond")) m_mode = cmBondPrices;
    else if (s.contains("price") && s.contains("stock")) m_mode = cmStockPrices;

}
void CycleWorker::prepareCycleData(QStringList &i_list)
{
    if (i_list.isEmpty())
    {
        emit signalError(QString("CycleWorker: cycle_data is empty, CYCLE MODE STOPED"));
        reset();
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
        reset();
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
        case cmBondPrices:
        case cmCoupons: {sign = "bond"; break;}

        case cmStockPrices:
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
            m_lastFigi = m_cycleData.first().figi;
            m_cycleData.removeFirst();
            break;
        }
        case cmHistory:
        {
            list << m_cycleData.first().uid;
            m_cycleData.removeFirst();
            break;
        }
        case cmStockPrices:
        case cmBondPrices:
        {
            for (int i=0; i<api_commonSettings.i_history.block_size; i++)
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
void CycleWorker::handleReplyData(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;
    const QJsonArray &j_arr = j_obj.constBegin().value().toArray();
    if (j_arr.isEmpty()) return;

    switch (m_mode)
    {
        case cmCoupons:     {parseCoupons(j_arr); break;}
        case cmDivs:        {parseDivs(j_arr); break;}
        case cmHistory:     {parseCandles(j_arr); break;}

        case cmBondPrices:
        case cmStockPrices:      {parsePrices(j_arr); break;}
        default: break;
    }
}
void CycleWorker::parseLastPrices(const QJsonObject &j_obj)
{
    if (j_obj.isEmpty()) return;
    const QJsonArray &j_arr = j_obj.constBegin().value().toArray();
    if (!j_arr.isEmpty()) parsePrices(j_arr);
}
QString CycleWorker::couponsFile()
{
    return QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(QString("coupons.xml"));
}
QString CycleWorker::divsFile()
{
    return QString("%1%2%3").arg(API_CommonSettings::appDataPath()).arg(QDir::separator()).arg(QString("divs.xml"));
}


//private funcs
void CycleWorker::parseCoupons(const QJsonArray &j_arr)
{
    //load old xml file
    qDebug()<<QString("CycleWorker::parseCoupons:  j_arr size %1").arg(j_arr.count());
    QDomDocument dom;
    QDomNode root_node;
    QString err = LStaticXML::getDomRootNode(CycleWorker::couponsFile(), root_node, QString("calendar"));
    if (root_node.isNull())
    {
        qWarning()<<"CycleWorker::parseCoupons WARNING - "<<err;
        root_node = dom.createElement("calendar");
    }

    //parse current JSON data
    BCoupon bc;
    for (int i=0; i<j_arr.count(); i++)
    {
        if (!j_arr.at(i).isObject()) continue;
        bc.fromJson(j_arr.at(i).toObject());
        if (bc.invalid()) qWarning("CycleWorker::parseCoupons WARNING - BCoupon INVALID");
        else bc.syncData(root_node, dom);
    }

    //write synchronized file
    LStaticXML::createDomHeader(dom);
    dom.appendChild(root_node);
    err = LFile::writeFile(CycleWorker::couponsFile(), dom.toString());
    if (!err.isEmpty()) emit signalError(err);
}
void CycleWorker::parseDivs(const QJsonArray &j_arr)
{
    //load old xml file
    qDebug()<<QString("CycleWorker::parseDivs:  j_arr size %1").arg(j_arr.count());
    QDomDocument dom;
    QDomNode root_node;
    QString err = LStaticXML::getDomRootNode(CycleWorker::divsFile(), root_node, QString("calendar"));
    if (root_node.isNull())
    {
        qWarning()<<"CycleWorker::parseCoupons WARNING - "<<err;
        root_node = dom.createElement("calendar");
    }

    //parse current JSON data
    SDiv d_rec(m_lastFigi);
    for (int i=0; i<j_arr.count(); i++)
    {
        if (!j_arr.at(i).isObject()) continue;
        d_rec.fromJson(j_arr.at(i).toObject());
        if (d_rec.invalid()) qWarning("CycleWorker::parseDivs WARNING - SDiv INVALID");
        else d_rec.syncData(root_node, dom);
    }

    //write synchronized file
    LStaticXML::createDomHeader(dom);
    dom.appendChild(root_node);
    err = LFile::writeFile(CycleWorker::divsFile(), dom.toString());
    if (!err.isEmpty()) emit signalError(err);
}
void CycleWorker::parseCandles(const QJsonArray &j_arr)
{

}
void CycleWorker::parsePrices(const QJsonArray &j_arr)
{
    for (int i=0; i<j_arr.count(); i++)
    {
        if (!j_arr.at(i).isObject()) continue;
        const QJsonObject &j_obj = j_arr.at(i).toObject();

        QString figi = j_obj.value("figi").toString();
        float price = InstrumentBase::floatFromJVBlock(j_obj.value("price"));
        if (price > 0 && !figi.isEmpty())
            emit signalCyclePrice(figi, price);
    }
}

