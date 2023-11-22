#include "reqpreparer.h"
#include "lhttpapirequester.h"
#include "apicommonsettings.h"
#include "lhttp_types.h"


#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>


// ApiReqPreparer
ApiReqPreparer::ApiReqPreparer(LHttpApiRequester *&req, QObject *parent)
    :LSimpleObject(parent),
      m_reqObj(req)
{

}
bool ApiReqPreparer::invalidReq() const
{
    return m_reqObj->metadata().keys().contains("err");
}
void ApiReqPreparer::prepare(QString src)
{
    m_reqObj->clearMetaData();

    QString token, baseURI;
    emit signalGetReqParams(token, baseURI);
    m_reqObj->addReqHeader(QString("Authorization"), QString("Bearer %1").arg(api_commonSettings.token));
    m_reqObj->addReqHeader(QString("accept"), QString("application/json"));
    m_reqObj->addReqHeader(QString("Content-Type"), QString("application/json"));
    m_reqObj->setUri(QString("%1.%2").arg(baseURI).arg(src));

    if (src.contains("OperationsService")) prepareReqOperations(src);
    else if (src.contains("BondBy")) prepareReqBondBy();
    else if (src.contains("ShareBy")) prepareReqShareBy();
    else if (src.contains("GetBondCoupons")) prepareReqCoupons();
    else if (src.contains("GetDividends")) prepareReqDivs();
    else if (src.contains("GetLastPrices")) prepareReqLastPrices();
    else if (src.contains("MarketDataService")) prepareReqMarket(src);
    else if (src.contains("OrdersService")) prepareReqOrders(src);


    emit signalMsg(QString("URL:   %1 \n").arg(m_reqObj->fullUrl()));
}
void ApiReqPreparer::prepareReqDivs()
{
    QString figi("figi");
    if (m_cycleData.isEmpty())
    {
        emit signalGetSelectedStockUID(figi);
        if (figi.isEmpty())
        {
            emit signalError("you must select some stock in the table");
            m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
            return;
        }
    }
    else figi = m_cycleData.first();

    emit signalMsg(QString("SELECTED FIGI: %1").arg(figi));
    m_reqObj->addMetaData("figi", figi);
    setMetaPeriod("divs");
}
void ApiReqPreparer::prepareReqCoupons()
{
    QString figi("figi");
    if (m_cycleData.isEmpty())
    {
        emit signalGetSelectedBondUID(figi);
        if (figi.isEmpty())
        {
            emit signalError("you must select some bond in the table");
            m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
            return;
        }
    }
    else figi = m_cycleData.first();

    emit signalMsg(QString("SELECTED FIGI: %1").arg(figi));
    m_reqObj->addMetaData("figi", figi);
    setMetaPeriod("coupons");
}
void ApiReqPreparer::prepareReqLastPrices()
{
    QStringList uid_list;
    if (m_cycleData.isEmpty())
    {
        emit signalGetSelectedBondUIDList(uid_list);
        if (uid_list.isEmpty())
        {
            emit signalGetSelectedStockUIDList(uid_list);
            if (uid_list.isEmpty())
            {
                emit signalError("you must select some bond or share in the table");
                m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
                return;
            }
        }
    }
    else uid_list.append(m_cycleData);

    emit signalMsg(QString("SELECTED UIDs: %1").arg(uid_list.count()));
    QJsonArray j_arr = QJsonArray::fromStringList(uid_list);
    m_reqObj->addMetaData_arr("instrumentId", j_arr);
}
void ApiReqPreparer::prepareReqOrders(const QString &src)
{
    m_reqObj->addMetaData("accountId", QString::number(api_commonSettings.user_id));
    if (src.contains("PostOrder"))
    {

    }
}
void ApiReqPreparer::prepareReqMarket(const QString &src)
{
    QString uid;
    if (m_cycleData.isEmpty())
    {
        emit signalGetSelectedBondUID(uid);
        if (uid.isEmpty())
        {
            emit signalGetSelectedStockUID(uid);
            if (uid.isEmpty())
            {
                emit signalError("you must select some bond or share in the table");
                m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
                return;
            }
        }
    }
    else uid = m_cycleData.first();

    emit signalMsg(QString("SELECTED UID: %1").arg(uid));
    m_reqObj->addMetaData("instrumentId", uid);

    if (src.contains("Book"))
    {
        quint16 dp = 0;
        emit signalGetPricesDepth(dp);
        m_reqObj->addMetaData("depth", QString::number(dp));
    }
    else if (src.contains("Candles"))
    {
        QString candle_size;
        emit signalGetCandleSize(candle_size);
        m_reqObj->addMetaData("interval", api_commonSettings.candle_sizes.value(candle_size));
        setMetaPeriod("prices");
    }
}
void ApiReqPreparer::prepareReqShareBy()
{
    QString uid;
    emit signalGetSelectedStockUID(uid);
    if (uid.isEmpty())
    {
        emit signalError("you must select some stock in the table");
        m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
        return;
    }

    emit signalMsg(QString("SELECTED UID: %1").arg(uid));
    m_reqObj->addMetaData("idType", "INSTRUMENT_ID_TYPE_UID");
    m_reqObj->addMetaData("id", uid);
}
void ApiReqPreparer::prepareReqBondBy()
{
    QString uid;
    emit signalGetSelectedBondUID(uid);
    if (uid.isEmpty())
    {
        emit signalError("you must select some bond in the table");
        m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
        return;
    }

    emit signalMsg(QString("SELECTED UID: %1").arg(uid));
    m_reqObj->addMetaData("idType", "INSTRUMENT_ID_TYPE_UID");
    m_reqObj->addMetaData("id", uid);
}
void ApiReqPreparer::prepareReqOperations(const QString &src)
{
    m_reqObj->addMetaData("currency", "RUB");
    m_reqObj->addMetaData("accountId", QString::number(api_commonSettings.user_id));

    if (src.contains("GetOperations"))
    {
        setMetaPeriod("events");
        m_reqObj->addMetaData("state", "OPERATION_STATE_EXECUTED");
    }
}
void ApiReqPreparer::setMetaPeriod(QString type)
{
    const API_CommonSettings::InstrumentHistory::HItem *h_item = NULL;
    if (type == "prices") h_item = &api_commonSettings.i_history.prices;
    else if (type == "divs") h_item = &api_commonSettings.i_history.divs;
    else if (type == "coupons") h_item = &api_commonSettings.i_history.coupons;
    else if (type == "events") h_item = &api_commonSettings.i_history.events;

    if (h_item)
    {
        m_reqObj->addMetaData("from", api_commonSettings.beginPoint(*h_item));
        m_reqObj->addMetaData("to", api_commonSettings.endPoint(*h_item));
        emit signalMsg(QString("PERIOD: %1").arg(h_item->toStr()));
    }
    else qWarning()<<QString("ApiReqPreparer::setMetaPeriod WARNING invalid period type - %1").arg(type);
}

