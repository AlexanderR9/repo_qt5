#include "lhtmlpagerequester.h"

#include <QWebEnginePage>
#include <QDebug>



// LHTMLPageRequester
LHTMLPageRequester::LHTMLPageRequester(QObject *parent)
    :LHTMLRequesterBase(parent),
      m_page(NULL),
      bad_request(false)
{
    setObjectName("lhtml_page_requester");

    m_page = new QWebEnginePage(this);
    connect(m_page, SIGNAL(loadFinished(bool)), this, SLOT(slotFinished(bool)));
    connect(m_page, SIGNAL(loadProgress(int)), this, SIGNAL(signalProgress(int)));

}
QString LHTMLPageRequester::title() const
{
    if (isBuzy()) return QString();
    if (badRequest()) return QString("???");
    return m_page->title();
}
void LHTMLPageRequester::startRequest()
{
    if (is_buzy)
    {
        emit signalError("page is buzy");
        return;
    }

    clearData();
    bool ok;
    checkUrl(ok);
    if (!ok)
    {
        emit signalFinished(false);
        return;
    }
    m_page->load(m_url);
}
void LHTMLPageRequester::slotFinished(bool ok)
{
    is_buzy = false;
    bad_request = !ok;
    emit signalFinished(ok);

    if (!ok) emit signalDataReady();
    else m_page->toPlainText([this](const QString &data){functorPlainData(data);});
}
void LHTMLPageRequester::clearData()
{
    html_data.clear();
    plain_data.clear();
    bad_request = false;
}
void LHTMLPageRequester::functorPlainData(const QString &data)
{
    plain_data = data.trimmed();
    m_page->toHtml([this](const QString &data){functorHtmlData(data);});
}
void LHTMLPageRequester::functorHtmlData(const QString &data)
{
    html_data = data.trimmed();
    emit signalDataReady();
}



