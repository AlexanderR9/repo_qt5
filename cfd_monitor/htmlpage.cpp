#include "htmlpage.h"
#include "lhtmlpagerequester.h"
#include "lhtmlrequester.h"
#include "lstatic.h"
#include "logpage.h"

#include <QTimer>

#define GET_DIV_TICKER  QString("get_divs")


//HtmlPage
HtmlPage::HtmlPage(QWidget *parent)
    :BasePage(parent),
      m_requester(NULL),
      m_timer(NULL),
      m_runingTime(0)
{
    setupUi(this);

    m_requester = new LHTMLPageRequester(this);
    connect(m_requester, SIGNAL(signalError(const QString&)), this, SIGNAL(signalError(const QString&)));
    connect(m_requester, SIGNAL(signalDataReady()), this, SLOT(slotDataReady()));
    connect(m_requester, SIGNAL(signalProgress(int)), this, SLOT(slotProgress(int)));

    m_timer = new QTimer(this);
    connect(m_timer, SIGNAL(timeout()), this, SLOT(slotTimer()));
    m_timer->setInterval(500);
}
void HtmlPage::slotTimer()
{
    m_runingTime++;
    infoLabel->setText(QString("request runing time: %1 sec.").arg(m_runingTime));
}
void HtmlPage::slotBreaked(int terminationStatus, int exitCode)
{
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

    resetPage(GET_DIV_TICKER);
    m_priceParser.updateInput(GET_DIV_TICKER, url);
    m_requester->setUrl(url);
    m_timer->start();
    m_requester->startRequest();
}
void HtmlPage::tryRequest(const QString &ticker)
{
    //qDebug()<<QString("HtmlPage::tryRequest for %1").arg(ticker);
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
    //qDebug("HtmlPage::getDivsFromPlainData()");
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

    //qDebug()<<QString("status=%1  action=%2").arg(m_requester->registerUserData());
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

//----------------------------------

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
            s = LStatic::strTrimLeft(s, 1).trimmed();
            //qDebug()<<QString("find data line_next [%1]").arg(s);
            int pos = s.indexOf(LStatic::spaceSymbol());
            if (pos > 0)
            {
                //qDebug()<<QString("   read price, symbol_pos=%1 s=[%2]").arg(pos).arg(s);
                m_price = s.left(pos).trimmed().toDouble(&ok);
            }
            else
            {
                //int len = s.length();
                //for (int k=0; k<len; k++)
                    //qDebug()<<QString("k=%0:  symbol=[%1]  code=%2").arg(k).arg(s[k]).arg(QChar(s[k]).unicode());
            }
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

    //qDebug()<<QString("HtmlWorker::execMetod2 - list size %1").arg(list.count());

    bool ok = false;
    for (int i=0; i<list.count(); i++)
    {
        QString s = list.at(i).trimmed();
        s = s.toLower();
        int pos = s.indexOf("price");
        if (pos < 0) continue;

        //qDebug()<<QString("find price word, line %1, s=[%2]").arg(i).arg(s);
        QString s2 = s.left(pos).trimmed();
        if (s2.right(6) == "target") {qDebug("has target"); continue;}

        s = LStatic::strTrimLeft(s, pos + 5).trimmed();
        pos = s.indexOf(LStatic::spaceSymbol());
        if (pos > 0 || s.length() < 7)
        {
            //qDebug()<<QString("   read price, symbol_pos=%1 s=[%2]").arg(pos).arg(s);
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

    m_err = QString("HtmlWorker: can't parse plain text for %1 (metod 3)").arg(m_ticker);
}
void HtmlWorker::execInvalidMetod()
{
    m_err = QString("HtmlWorker: invalid parsing metod %1, url=[%2]").arg(parseMetod()).arg(m_url);
}
void HtmlWorker::trimData(const QString &data, QStringList &list)
{
    list.clear();
    list = LStatic::trimSplitList(data);

    for (int i=0; i<list.count(); i++)
    {
        QString s = LStatic::removeLongSpaces(list.at(i));
        list[i] = s;
    }
}


