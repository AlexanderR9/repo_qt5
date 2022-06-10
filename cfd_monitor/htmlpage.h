#ifndef HTMLPAGE_H
#define HTMLPAGE_H

#include "basepage.h"

#include "ui_htmlpage.h"

#include <QWebEnginePage>
#include <QPair>

//parser prices
class HtmlWorker
{
public:
    enum PriceParseMetod {ppmSmartLab = 401, ppmFinviz, ppmInvesting, ppmUnknown = -1};

    HtmlWorker() {}

    void updateInput(const QString&, const QString&);
    void tryParsePrice(const QString&);

    inline bool invalid() const {return (m_price < 0);}
    inline QString err() const {return m_err;}
    inline QPair<QString, double> lastPrice() const {return QPair<QString, double>(m_ticker, m_price);}

protected:
    QString     m_ticker;
    QString     m_url;
    double      m_price;
    QString     m_err;

    int parseMetod() const;
    void execMetod1(const QString&);
    void execMetod2(const QString&);
    void execMetod3(const QString&);
    void execInvalidMetod();

private:
    void trimData(const QString&, QStringList&);


};



class LHTMLPageRequester;
class LHTMLRequester;
class QTimer;

//HtmlPage
class HtmlPage : public BasePage, Ui::HtmlPage
{
    Q_OBJECT
public:
    HtmlPage(QWidget*);
    virtual ~HtmlPage() {}

    QString iconPath() const {return QString(":/icons/images/html.png");}
    QString caption() const {return QString("HTML");}

    void updatePage() {}
    void tryRequest(const QString&);
    void tryHTTPRequest(const QString&);
    void getPriceFromPlainData();

protected:
    LHTMLPageRequester *m_requester;
    LHTMLRequester     *http_requester;
    HtmlWorker          m_priceParser;
    QTimer              *m_timer;
    int                 m_runingTime;

    void resetPage(const QString&);

protected slots:
    void slotDataReady(); //выполняется когда запрос полностью завершен, независимо от результата выполнения
    void slotProgress(int); //выполняется в процессе выполнения запроса
    void slotBreaked(int, int);
    void slotRequestFinished(bool);
    void slotTimer();

signals:
    void signalGetUrlByTicker(const QString&, QString&);
    void signalNewPrice(QString, double);

};


#endif //HTMLPAGE_H


