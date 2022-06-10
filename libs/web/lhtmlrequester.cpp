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
    connect(m_manager, SIGNAL(networkAccessibleChanged(QNetworkAccessManager::NetworkAccessibility)), this, SLOT(slotAccessibleChanged()));
    connect(m_manager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(slotAuthenticationRequired()));
    connect(m_manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)), this, SLOT(slotSSLErrors()));


    qDebug()<<QString("req header list %1").arg(m_request.rawHeaderList().count());
    initHttpHeaders();

}
void LHTMLRequester::slotAccessibleChanged()
{
    qDebug()<<QString("LHTMLRequester::slotAccessibleChanged()");
}
void LHTMLRequester::slotAuthenticationRequired()
{
    qDebug()<<QString("LHTMLRequester::slotAuthenticationRequired()");
}
void LHTMLRequester::slotSSLErrors()
{
    qDebug()<<QString("LHTMLRequester::slotSSLErrors()");
}
void LHTMLRequester::initHttpHeaders()
{
    m_request.setRawHeader("Accept", "text/html");
    m_request.setRawHeader("Accept-Enconding", "identity");
    m_request.setRawHeader("Accept-Charset", "utf-8");
    m_request.setRawHeader("User-Agent", "My app name v0.1");
    m_request.setHeader(QNetworkRequest::ContentTypeHeader,  "application/x-www-form-urlencoded");
    m_request.setRawHeader("Accept-Language", "ru-RU,ru;q=0.8,en-US;q=0.5,en;q=0.3");
    m_request.setRawHeader("Connection", "keep-alive");


}
void LHTMLRequester::startRequest()
{
    qDebug()<<QString("LHTMLRequester::startRequest() URL=[%1]").arg(m_url);
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
    qDebug()<<QString("req header list %1").arg(m_request.rawHeaderList().count());

//    qDebug("LHTMLRequester::startRequest() 4");
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

    if (reply->error() != QNetworkReply::NoError)
    {
        emit signalError(QString("request result code: %1").arg(reply->error()));
        emit signalFinished(false);
        return;
    }

    m_replyBuffer = reply->readAll();
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
    LHTMLRequesterBase::checkUrl(ok);
    if (!ok) return;


    ok = false;
    /*
    if (!m_request.url().isValid())
    {
        emit signalError(QString("URL is invalid: [%1]").arg(m_url));
        return;
    }
    */
    ok = true;
}




