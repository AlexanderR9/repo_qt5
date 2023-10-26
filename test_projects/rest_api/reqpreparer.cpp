#include "reqpreparer.h"
#include "lhttpapirequester.h"
#include "apicommonsettings.h"
#include "lhttp_types.h"


#include <QJsonObject>
#include <QJsonArray>


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

    if (src.contains("OperationsService")) prepareReqOperations();
    else if (src.contains("BondBy")) prepareReqBondBy();
    else if (src.contains("ShareBy")) prepareReqShareBy();
    else if (src.contains("GetBondCoupons")) prepareReqCoupons();
    else if (src.contains("GetDividends")) prepareReqDivs();
    else if (src.contains("GetLastPrices")) prepareReqLastPrices();
    else if (src.contains("MarketDataService")) prepareReqMarket(src);

    emit signalMsg(QString("URL:   %1 \n").arg(m_reqObj->fullUrl()));
}
void ApiReqPreparer::prepareReqDivs()
{
    QString figi("figi");
    emit signalGetSelectedStockUID(figi);
    if (figi.isEmpty())
    {
        emit signalError("you must select some stock in the table");
        m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
        return;
    }

    emit signalMsg(QString("SELECTED UID: %1").arg(figi));
    m_reqObj->addMetaData("figi", figi);
    m_reqObj->addMetaData("from", api_commonSettings.beginPoint(api_commonSettings.i_history.divs));
    m_reqObj->addMetaData("to", api_commonSettings.endPoint(api_commonSettings.i_history.divs));
    emit signalMsg(QString("PERIOD: %1").arg(api_commonSettings.i_history.divs.toStr()));
}
void ApiReqPreparer::prepareReqCoupons()
{
    QString figi("figi");
    emit signalGetSelectedBondUID(figi);
    if (figi.isEmpty())
    {
        emit signalError("you must select some bond in the table");
        m_reqObj->addMetaData("err", QString::number(hreWrongReqParams));
        return;
    }

    emit signalMsg(QString("SELECTED UID: %1").arg(figi));
    m_reqObj->addMetaData("figi", figi);
    m_reqObj->addMetaData("from", api_commonSettings.beginPoint(api_commonSettings.i_history.coupons));
    m_reqObj->addMetaData("to", api_commonSettings.endPoint(api_commonSettings.i_history.coupons));
    emit signalMsg(QString("PERIOD: %1").arg(api_commonSettings.i_history.coupons.toStr()));
}
void ApiReqPreparer::prepareReqLastPrices()
{
    QStringList uid_list;
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

    emit signalMsg(QString("SELECTED UIDs: %1").arg(uid_list.count()));
    QJsonArray j_arr = QJsonArray::fromStringList(uid_list);
    m_reqObj->addMetaData_arr("instrumentId", j_arr);
}
void ApiReqPreparer::prepareReqMarket(const QString &src)
{
    QString uid;
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
        m_reqObj->addMetaData("from", api_commonSettings.beginPoint(api_commonSettings.i_history.prices));
        m_reqObj->addMetaData("to", api_commonSettings.endPoint(api_commonSettings.i_history.prices));
        emit signalMsg(QString("HISTORY_PERIOD: %1").arg(api_commonSettings.i_history.prices.toStr()));
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
void ApiReqPreparer::prepareReqOperations()
{
    m_reqObj->addMetaData("currency", "RUB");
    m_reqObj->addMetaData("accountId", QString::number(api_commonSettings.user_id));
}

