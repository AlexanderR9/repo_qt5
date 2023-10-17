#include "htmlpage.h"
#include "lhtmlpagerequester.h"
#include "lhtmlrequester.h"
#include "lstring.h"
#include "logpage.h"

#include <QTimer>

#define GET_DIV_TICKER          QString("get_divs")
#define REQUEST_TIMEOUT         90 //sec


//HtmlPage
HtmlPage::HtmlPage(QWidget *parent)
    :BasePage(parent),
      m_requester(NULL),
      m_timer(NULL),
      m_runingTime(0)
{
    setupUi(this);

    m_requester = new LHTMLPageRequester(this);
    connect(m_requester, SIGNAL(signalError(const QString&)), this, SLOT(slotError(const QString&)));
    connect(m_requester, SIGNAL(signalDataReady()), this, SLOT(slotDataReady()));
    connect(m_requester, SIGNAL(signalProgress(int)), this, SLOT(slotProgress(int)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    m_timer->setInterval(1000);
}
void HtmlPage::slotError(const QString &err)
{
    emit signalError(err);
    sendLog(err, 2);
}
void HtmlPage::slotTimer()
{
    m_runingTime++;
    infoLabel->setText(QString("request runing time: %1 sec.").arg(m_runingTime));
    if (m_runingTime > REQUEST_TIMEOUT) m_requester->breakTimeout();
}
void HtmlPage::slotBreaked(int terminationStatus, int exitCode)
{
    Q_UNUSED(terminationStatus);
    Q_UNUSED(exitCode);
    qWarning()<<QString("HtmlPage::slotBreaked WARNING - terminationStatus=%1  exitCode=%2").arg(terminationStatus).arg(exitCode);
    m_timer->stop();
}
void HtmlPage::slotGetDivData(const QString &url)
{
    if (m_requester->isBuzy())
    {
        emit signalError("requester is buzy");
        sendLog("get div data", 2);
        return;
    }
    else resetPage(GET_DIV_TICKER);

    m_priceParser.updateInput(GET_DIV_TICKER, url);
    m_requester->setUrl(url);
    m_timer->start();
    m_requester->startRequest();
}
void HtmlPage::tryRequest(const QString &ticker)
{
    if (m_requester->isBuzy())
    {
        emit signalError("requester is buzy");
        sendLog(ticker, 2);
        return;
    }

    resetPage(ticker);
    if (ticker.isEmpty())
    {
        emit signalError("current ticker is empty");
        sendLog(QString("???"), 2);
        return;
    }

    QString url;
    emit signalGetUrlByTicker(ticker, url);

    m_requester->setUrl(url);
    m_priceParser.updateInput(ticker, url);
    m_timer->start();
    m_requester->startRequest();
}
void HtmlPage::getPriceFromPlainData()
{
    m_priceParser.tryParsePrice(m_requester->plainData());
    if (m_priceParser.invalid())
    {
        emit signalError(m_priceParser.err());
        sendLog(QString("Can't parsing HTML (%1)").arg(m_priceParser.curTicker()), 1);
    }
    else
    {
        QPair<QString, double> pair(m_priceParser.lastPrice());
        emit signalNewPrice(pair.first, pair.second);
    }
}
void HtmlPage::getDivsFromPlainData()
{
    emit signalDivDataReceived(m_requester->plainData());
}
void HtmlPage::resetPage(const QString &ticker)
{
    groupBox->setTitle(QString("HTML code, loading (%1%)").arg(-1));
    infoLabel->setText(QString("TICKER = [%1]").arg(ticker));
    textEdit->clear();
    m_runingTime = 0;
}
void HtmlPage::slotDataReady()
{
    m_timer->stop();
    groupBox->setTitle(QString("HTML code, execuded!"));

    QString s;
    if (m_requester->badRequest())
    {
        s = QString("RESULT=[FAULT]");
        sendLog(m_priceParser.curTicker(), 1);
    }
    else
    {
        s = QString("RESULT=[OK]  HTML_SIZE=[%1]  TEXT_SIZE=[%2]").arg(m_requester->htmlDataSize()).arg(m_requester->plainDataSize());
        textEdit->setPlainText(m_requester->plainData());
        emit signalMsg("Ok!");
        sendLog(m_priceParser.curTicker(), 0);

        if (isDivRequest()) getDivsFromPlainData();
        else getPriceFromPlainData();
    }
    infoLabel->setText(QString("%1 - %2").arg(infoLabel->text()).arg(s));
}
void HtmlPage::sendLog(const QString &ticker, int result)
{
    LogStruct log(amtHtmlPage, result);
    if (ticker.length() > 5) log.msg = ticker;
    else log.msg = QString("HTTP request (%1)").arg(ticker);
    emit signalSendLog(log);
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
}
bool HtmlPage::isDivRequest() const
{
    return (m_priceParser.curTicker() == GET_DIV_TICKER);
}





//HtmlWorker
void HtmlWorker::updateInput(const QString &ticker, const QString &url)
{
    m_ticker = ticker.trimmed();
    m_url = url.trimmed();
    m_price = -1;
    m_err.clear();
}
void HtmlWorker::tryParsePrice(const QString &data)
{
    if (data.trimmed().isEmpty())
    {
        m_err = "HtmlWorker: parsing data is empty.";
        return;
    }
    if (m_ticker.isEmpty())
    {
        m_err = "HtmlWorker: ticker is empty.";
        return;
    }
    if (m_url.isEmpty())
    {
        m_err = "HtmlWorker: url is empty.";
        return;
    }

//-------------------------------------------------------------

    switch (parseMetod())
    {
        case ppmSmartLab:   {execMetod1(data); break;}
        case ppmFinviz:     {execMetod2(data); break;}
        case ppmInvesting:  {execMetod3(data); break;}
        default:            {execInvalidMetod(); break;}
    }
}
int HtmlWorker::parseMetod() const
{
    if (m_url.contains(QString("smart-lab.ru"))) return ppmSmartLab;
    if (m_url.contains(QString("finviz.com"))) return ppmFinviz;
    if (m_url.contains(QString("investing.com"))) return ppmInvesting;
    return ppmUnknown;
}
void HtmlWorker::execMetod1(const QString &data) //smart-lab
{
    QStringList list;
    trimData(data, list);

    bool ok = false;
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        if (s.left(1) == "$" && s.right(1) == "%")
        {
            s = LString::strTrimLeft(s, 1).trimmed();
            int pos = s.indexOf(LString::spaceSymbol());
            if (pos > 0) m_price = s.left(pos).trimmed().toDouble(&ok);
            break;
        }
    }

    if (!ok)
    {
        m_price = -1;
        m_err = QString("HtmlWorker: can't parse plain text for %1 (metod 1)").arg(m_ticker);
    }
}
void HtmlWorker::execMetod2(const QString &data) //finviz
{
    QStringList list;
    trimData(data, list);

    bool ok = false;
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        s = s.toLower();
        int pos = s.indexOf("price");
        if (pos < 0) continue;

        QString s2 = s.left(pos).trimmed();
        if (s2.right(6) == "target") {qDebug("has target"); continue;}

        s = LString::strTrimLeft(s, pos + 5).trimmed();
        pos = s.indexOf(LString::spaceSymbol());
        if (pos > 0 || s.length() < 7)
        {
            m_price = s.left(pos).trimmed().toDouble(&ok);
            break;
        }
    }

    if (!ok)
    {
        m_price = -1;
        m_err = QString("HtmlWorker: can't parse plain text for %1 (metod 2)").arg(m_ticker);
    }
}
void HtmlWorker::execMetod3(const QString &data) //investing
{
    QStringList list;
    trimData(data, list);
    int n = list.count();
    for (int i=0; i<n; i++)
    {
        if (list.first().contains(QString("(%1)").arg(curTicker()))) break;
        list.removeFirst();
    }
    if (list.isEmpty())
    {
        m_price = -1;
        m_err = QString("HtmlWorker: can't parse plain text for %1 (metod 3), trimmed html data is empty ").arg(m_ticker);
        return;
    }

    bool ok;
    foreach (QString v, list)
    {
        if (v.length() > 8) continue;
        if (v.contains(",")) v.replace(",", ".");
        double p = v.toDouble(&ok);
        if (ok) {m_price = p; qDebug()<<QString("price %1 ok!").arg(m_price); break;}
    }

    if (m_price < 0)
        m_err = QString("HtmlWorker: can't parse plain text for %1 (metod 3), not found correct price").arg(m_ticker);
}
void HtmlWorker::execInvalidMetod()
{
    m_err = QString("HtmlWorker: invalid parsing metod %1, url=[%2]").arg(parseMetod()).arg(m_url);
}
void HtmlWorker::trimData(const QString &data, QStringList &list)
{
    list.clear();
    list = LString::trimSplitList(data);
    for (int i=0; i<list.count(); i++)
    {
        QString s = LString::removeLongSpaces(list.at(i));
        list[i] = s;
    }
}


