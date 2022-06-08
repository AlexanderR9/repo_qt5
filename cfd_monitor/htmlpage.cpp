#include "htmlpage.h"
#include "lhtmlpagerequester.h"


//HtmlPage
HtmlPage::HtmlPage(QWidget *parent)
    :BasePage(parent),
      m_requester(NULL)
{
    setupUi(this);

    m_requester = new LHTMLPageRequester(this);
    connect(m_requester, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_requester, SIGNAL(signalDataReady()), this, SLOT(slotDataReady()));
    connect(m_requester, SIGNAL(signalProgress(int)), this, SLOT(slotProgress(int)));

    //connect(m_requester, SIGNAL(renderProcessTerminated(RenderProcessTerminationStatus, int)),
      //      this, SLOT(slotBreaked(int, int)));

    //QIcon icon(QString(":/icons/images/ball_gray.svg"));
    //groupBox->setWindowIcon(icon);
}
void HtmlPage::slotBreaked(int terminationStatus, int exitCode)
{

}
void HtmlPage::tryRequest(const QString &ticker)
{
    if (m_requester->isBuzy())
    {
        emit signalError("requester is buzy");
        return;
    }

    resetPage(ticker);

    if (ticker.isEmpty())
    {
        emit signalError("current ticker is empty");
        return;
    }


    QString url;
    emit signalGetUrlByTicker(ticker, url);
    m_requester->setUrl(url);
    m_requester->startRequest();

}
void HtmlPage::resetPage(const QString &ticker)
{
    groupBox->setTitle(QString("HTML code, loading (%1%)").arg(-1));
    infoLabel->setText(QString("TICKER = [%1]").arg(ticker));
    textEdit->clear();
}
void HtmlPage::slotDataReady()
{
    groupBox->setTitle(QString("HTML code, execuded!"));

    QString s;
    if (m_requester->badRequest())
    {
        s = QString("RESULT=[FAULT]");
    }
    else
    {
        s = QString("RESULT=[OK]  HTML_SIZE=[%1]  TEXT_SIZE=[%2]").arg(m_requester->htmlDataSize()).arg(m_requester->plainDataSize());
        textEdit->setPlainText(m_requester->plainData());
        emit signalMsg("Ok!");
    }
    infoLabel->setText(QString("%1 - %2").arg(infoLabel->text()).arg(s));
}
void HtmlPage::slotProgress(int p)
{
    if (!m_requester->isBuzy()) return;

    groupBox->setTitle(QString("HTML code, loading (%1%) .....").arg(p));
    if (textEdit->toPlainText().isEmpty())
    {
        textEdit->append(QString("URL = [%1]").arg(m_requester->url()));
        textEdit->append(QString("waiting ........"));
    }

    //qDebug()<<QString("status=%1  action=%2").arg(m_requester->registerUserData());
}


