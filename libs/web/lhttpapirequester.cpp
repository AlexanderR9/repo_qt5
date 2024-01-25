#include "lhttpapirequester.h"
#include "lhttp_types.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QStringList>
#include <QByteArray>
#include <QJsonDocument>



////////////////LHttpApiRequester///////////////////////
LHttpApiRequester::LHttpApiRequester(QObject *parent)
    :LSimpleObject(parent),
      m_manager(NULL),
      m_request(NULL),
      m_protocolType(hptHttp),
      m_apiServer(QString()),
      m_uri(QString()),
      m_buzy(false)
{
    setObjectName("lhttpapirequester");
    initNetObjects();
    m_lastReply.reset();
    clearMetaData();
}
void LHttpApiRequester::initNetObjects()
{
    m_request = new QNetworkRequest();
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotFinished(QNetworkReply*)));
    connect(m_manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(slotAuthenticationRequired()));
    connect(m_manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(slotSSLErrors()));
}
void LHttpApiRequester::reinitReq()
{
    if (m_request) {delete m_request; m_request = NULL;}
    m_request = new QNetworkRequest();
}
void LHttpApiRequester::slotFinished(QNetworkReply *reply)
{
    m_buzy = false;
    int code = hreUnknown;
    if (reply)
    {
        emit signalMsg(QString("LHttpApiRequester::slotFinished  err_code=%1").arg(reply->error()));

        m_lastReply.takeHeaders(reply);
        m_lastReply.takeData(reply);
        if (!m_lastReply.isOk())
        {
            emit signalError(QString("QNetworkReply result code: %1").arg(m_lastReply.result_code));
            code = hreServerErr;
        }
        else code = hreOk;
    }
    else
    {
        emit signalError("reply is NULL");
        code = hreReplyNull;
    }
    emit signalFinished(code);
}
void LHttpApiRequester::start(int metod)
{
    m_lastReply.reset();
    QString err(validity());
    if (!err.isEmpty())
    {
        emit signalError(err);
        emit signalFinished(hreWrongReqParams);
        return;
    }

    m_request->setUrl(fullUrl());
    switch (metod)
    {
        case hrmGet:
        {
            m_buzy = true;
            m_manager->get(*m_request);
            break;
        }
        case hrmPost:
        {
            m_buzy = true;
            m_manager->post(*m_request, QJsonDocument(m_metadata).toJson());
            break;
        }
        default:
        {
            emit signalError(QString("Invalid HTTP metod: %1").arg(metod));
            emit signalFinished(hreInvalidMetod);
            return;
        }
    }
}
void LHttpApiRequester::destroyObj()
{
    m_lastReply.reset();
    clearMetaData();
    if (m_request) {delete m_request; m_request = NULL;}
    if (m_manager) {delete m_manager; m_manager = NULL;}
}
void LHttpApiRequester::getReqHeaders(QStringList &list)
{
    list.clear();
    if (!m_request)
    {
        emit signalError(QString("getReqHeaders: REQUEST_OBJ is NULL"));
        return;
    }

    QList<QByteArray> data = m_request->rawHeaderList();
    foreach (const QByteArray& v, data)
    {
        QString header_name(v);
        QString header_value(m_request->rawHeader(v));
        list.append(QString("%1: %2").arg(header_name).arg(header_value));
    }
}
void LHttpApiRequester::addReqHeader(QString key, QString value)
{
    if (!m_request)
    {
        emit signalError(QString("addReqHeader: REQUEST_OBJ is NULL"));
        return;
    }
    m_request->setRawHeader(key.toLatin1(), value.toLatin1());
}
QString LHttpApiRequester::httpType(int t)
{
    switch (t)
    {
        case hptHttp: return QString("http://");
        case hptHttps: return QString("https://");
        default: break;
    }
    return "???";
}
QString LHttpApiRequester::fullUrl() const
{
    QString url(httpType(m_protocolType));
    url.append(m_apiServer);
    if (!m_uri.isEmpty())
        url = QString("%1/%2").arg(url).arg(m_uri);
    return url.trimmed();
}
QString LHttpApiRequester::validity() const
{
    if (!fullUrl().contains("http")) return QString("Invalid HTTP type: %1").arg(m_protocolType);
    if (m_apiServer.isEmpty()) return QString("Invalid API server domen: IS_EMPTY");
    if (m_apiServer.length() < 5 || !m_apiServer.contains(".") || m_apiServer.contains("/"))
        return QString("Invalid API server domen - %1").arg(m_apiServer);
    return QString();
}


////////////////LHttpApiReplyData///////////////////////
void LHttpApiReplyData::takeHeaders(const QNetworkReply *reply)
{
    if (!reply) return;

    QList<QByteArray> raw_headers = reply->rawHeaderList();
    foreach (const QByteArray& v, raw_headers)
    {
        QString header_name(v);
        QString header_value(reply->rawHeader(v));
        headers.append(QString("%1: %2").arg(header_name).arg(header_value));
    }
}
void LHttpApiReplyData::takeData(QNetworkReply *reply)
{
    if (!reply) return;

    result_code = int(reply->error());
    if (isOk())
        data = QJsonDocument::fromJson(reply->readAll()).object();
}
bool LHttpApiReplyData::isOk() const
{
    return (hreOk == 0);
}



