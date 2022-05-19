#include "tgsender.h"

#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QJsonDocument>



LTGSender::LTGSender(const QString &token, QObject *parent)
    :QObject(parent),
    m_botToken(token),
    m_request(NULL),
    m_netManager(NULL)
{
    setObjectName("tg_sender");
    initNetObjects();

}
LTGSender::~LTGSender()
{
    if (m_request)
    {
        delete m_request;
        m_request = NULL;
    }
}
void LTGSender::initNetObjects()
{
    m_request = new QNetworkRequest();
    m_request->setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_netManager = new QNetworkAccessManager(this);
    connect(m_netManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotRequestFinished(QNetworkReply*)));
}
QString LTGSender::apiMetodByReqCode(int code)
{
    switch (code)
    {
        case tgrcGetMe: return QString("getMe");
        case tgrcSendTextMsg: return QString("sendMessage");
        case tgrcGetUpdates: return QString("getUpdates");
        default: break;
    }
    return QString();
}
QString LTGSender::requestUrl(const QString &api_metod) const
{
    return QString("%1/bot%2/%3").arg(tgUrlBase()).arg(m_botToken).arg(api_metod);
}
void LTGSender::sendJsonRequest(const QJsonObject &json_obj, int req_code)
{
    m_err.clear();
    QString api_method = LTGSender::apiMetodByReqCode(req_code);
    if (api_method.trimmed().isEmpty())
    {
        m_err = QString("request code invalid: %1").arg(req_code);
        return;
    }

    qDebug() << QString("SEND REQUEST (%1)").arg(api_method);
    m_request->setUrl(requestUrl(api_method));
    m_netManager->post(*m_request, QJsonDocument(json_obj).toJson());
}
void LTGSender::slotRequestFinished(QNetworkReply *reply)
{
    m_err.clear();
    if (reply)
    {
        QJsonObject result = QJsonDocument::fromJson(reply->readAll()).object();
        if (result.count() < 2)
        {
            m_err = QString("answered json invalid, size=%1").arg(result.count());
            emit signalFinishedFault();
            return;
        }
        QStringList keys = result.keys();
        if (!keys.contains("ok"))
        {
            m_err = QString("answered json invalid, not found key [ok]");
            emit signalFinishedFault();
            return;
        }
        if (!keys.contains("result"))
        {
            m_err = QString("answered json invalid, not found key [result]");
            emit signalFinishedFault();
            return;
        }
        if (!result["ok"].toBool())
        {
            m_err = QString("answered json invalid, value[ok]=false");
            emit signalFinishedFault();
            return;
        }
        if (!result["result"].isObject() && !result["result"].isArray())
        {
            m_err = QString("answered json invalid, value[result] is not json_object");
            emit signalFinishedFault();
            return;
        }

        if (result["result"].isArray()) emit signalJArrReceived(result["result"].toArray());
        else emit signalJsonReceived(result["result"].toObject());

        reply->deleteLater();
    }
    else
    {
        m_err = QString("answered json invalid, reply is null");
        emit signalFinishedFault();
    }
}



