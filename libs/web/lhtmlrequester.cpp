 #include "lhtmlrequester.h"

 #include <QDebug>
 #include <QByteArray>
 #include <QNetworkAccessManager>
 #include <QNetworkReply>
 
// LHTMLRequester
LHTMLRequester::LHTMLRequester(QObject *parent)
    :LHTMLRequesterBase(parent),
    m_manager(NULL)
{
    setObjectName("lhtml_requester");

    m_replyBuffer.clear();
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotFinished(QNetworkReply*)));
}
void LHTMLRequester::initHttpHeaders()
{
    m_request.setRawHeader("Accept", "text/html");
    m_request.setRawHeader("Accept-Enconding", "identity");
    m_request.setRawHeader("Accept-Charset", "utf-8");
}
void LHTMLRequester::startRequest()
{
    if (is_buzy)
    {
        emit signalError("requester is buzy");
        return;
    }


    bool ok;
    checkUrl(ok);
    if (!ok) 
    {
        emit signalFinished(false);
        return;
    }

    is_buzy = true;
    m_manager->get(m_request);
}
void LHTMLRequester::slotFinished(QNetworkReply *reply)
{
    is_buzy = false;

    m_replyBuffer.clear();
    if (!reply) 
    {
        emit signalError("reply is null");
        emit signalFinished(false);
        return;
    }

    if (reply->error() != 0)
    {
        emit signalError(QString("request result code: %1").arg(reply->error()));
    }
    else m_replyBuffer = reply->readAll();

    emit signalFinished(true);
    reply->deleteLater();
}
void LHTMLRequester::getHtmlData(QString &data)
{
    data.clear();
    if (replyEmpty()) return;
    data.append(m_replyBuffer);
}
void LHTMLRequester::checkUrl(bool &ok)
{
    LHTMLRequester::checkUrl(ok);
    if (!ok) return;

    ok = false;
    if (!m_request.url().isValid())
    {
        emit signalError(QString("URL is invalid: [%1]").arg(m_url));
        return;
    }
    ok = true;
}




