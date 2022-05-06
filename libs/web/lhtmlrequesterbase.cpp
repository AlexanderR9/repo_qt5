 #include "lhtmlrequesterbase.h"

 #include <QDebug>
 #include <QByteArray>
 #include <QNetworkAccessManager>
 #include <QNetworkReply>
 
// LHTMLRequesterBase
LHTMLRequesterBase::LHTMLRequesterBase(QObject *parent)
    :QObject(parent),
    m_url(QString()),
    is_buzy(false)
{
    setObjectName("lhtml_requester_base");

}
void LHTMLRequesterBase::setUrl(const QString &url)
{
    m_url = url.trimmed();
}
void LHTMLRequesterBase::checkUrl(bool &ok)
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
    ok = true;
}


