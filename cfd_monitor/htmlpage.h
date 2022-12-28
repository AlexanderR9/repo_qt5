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
    inline const QString& curTicker() const {return m_ticker;}

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

protected:
    LHTMLPageRequester  *m_requester;
    HtmlWorker           m_priceParser;
    QTimer              *m_timer;
    int                  m_runingTime;

    void resetPage(const QString&);
    void sendLog(const QString&, int);
    void getPriceFromPlainData();
    void getDivsFromPlainData();
    bool isDivRequest() const;

protected slots:
    void slotDataReady(); //выполняется когда запрос полностью завершен, независимо от результата выполнения
    void slotProgress(int); //выполняется в процессе выполнения запроса
    void slotBreaked(int, int);
    void slotTimer();
    void slotError(const QString&);

signals:
    void signalGetUrlByTicker(const QString&, QString&);
    void signalNewPrice(QString, double);
    void signalDivDataReceived(const QString&);

public slots:
    void slotGetDivData(const QString&);

};


#endif //HTMLPAGE_H


