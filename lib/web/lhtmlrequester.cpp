 #include "lhtmlrequester.h"

 #include <QDebug>
 #include <QByteArray>
 #include <QNetworkAccessManager>
 #include <QNetworkReply>
 
// LHTMLRequester
LHTMLRequester::LHTMLRequester(QObject *parent)
    :QObject(parent),
    m_manager(NULL),
    m_url(QString())
{
    setObjectName("lhtml_requester");

    m_replyBuffer.clear();
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(slotFinished(QNetworkReply*)));

//    initHttpHeaders();
}
void LHTMLRequester::initHttpHeaders()
{
    m_request.setRawHeader("Accept", "text/html");
    m_request.setRawHeader("Accept-Enconding", "identity");
    m_request.setRawHeader("Accept-Charset", "utf-8");
}
void LHTMLRequester::startRequest()
{
    //qDebug()<<QString("HTMLRequester::startRequest()  url=[%1]").arg(m_url);
    bool ok;
    checkUrl(ok);
    if (!ok) 
    {
        emit signalFinished();
        return;
    }

    //qDebug("get....");
    m_manager->get(m_request);
}
void LHTMLRequester::slotFinished(QNetworkReply *reply)
{
    //qDebug("HTMLRequester::slotFinished");
    m_replyBuffer.clear();
    if (!reply) 
    {
        emit signalError("reply is null");
        emit signalFinished();
        return;
    }

    if (reply->error() != 0)
    {
        emit signalError(QString("request result code: %1").arg(reply->error()));
    }
    else
    {
        m_replyBuffer = reply->readAll();
        //qDebug()<<QString("reply: buff size: %1, err=%2").arg(m_replyBuffer.size()).arg(reply->error());
    }

    emit signalFinished();
    reply->deleteLater();
}
void LHTMLRequester::getHtmlData(QString &data)
{
    data.clear();
    if (replyEmpty()) return;
    data.append(m_replyBuffer);
}
void LHTMLRequester::setUrl(const QString &url)
{
    m_url = url.trimmed();
    m_request.setUrl(QUrl(m_url));
}
void LHTMLRequester::checkUrl(bool &ok)
{
    ok = false;
    if (m_url.isEmpty())
    {
        emit signalError("URL is empty!");
        return;
    }
    if (m_url.left(4) != "http")
    {
        emit signalError("URL must begin \"http\"!");
        return;
    }
    if (!m_request.url().isValid())
    {
        emit signalError(QString("URL is invalid: [%1]").arg(m_url));
        return;
    }
    ok = true;
}




