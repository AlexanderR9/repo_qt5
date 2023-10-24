#ifndef APIREQPAGE_H
#define APIREQPAGE_H

#include "lsimplewidget.h"

class LHttpApiRequester;


//APIReqPage
class APIReqPage : public LSimpleWidget
{
    Q_OBJECT
public:
    APIReqPage(QWidget*);
    virtual ~APIReqPage() {}

    QString iconPath() const {return QString(":/icons/images/b_scale.svg");}
    QString caption() const {return QString("API request");}
    virtual void resetPage();
    void trySendReq();
    void autoStartReq(QString);
    void setServerAPI(int, const QString&);
    void checkReply();
    bool replyOk() const;
    void setExpandLevel(int);
    inline void setPrintHeaders(bool b) {m_printHeaders = b;}
    bool requesterBuzy() const;

protected:
    LListWidgetBox      *m_sourceBox;
    LTreeWidgetBox      *m_replyBox;
    LHttpApiRequester   *m_reqObj;
    bool                 m_printHeaders;

    void initWidgets();
    void initSources();
    void prepareReq(int);
    void handleReplyData();
    void saveBondsFile();
    void saveStocksFile();
    void parseUserID();
    void printHeaders(QString s = "req"); //param - req or resp

signals:
    void signalFinished(int);
    void signalGetReqParams(QString&, QString&); //try get tocken and base_uri
    void signalGetSelectedBondUID(QString&);
    void signalGetSelectedStockUID(QString&);
    void signalGetSelectedBondUIDList(QStringList&);
    void signalGetSelectedStockUIDList(QStringList&);
    void signalGetPricesDepth(quint16&);
    void signalGetCandleSize(QString&);
    void signalLoadPositions(const QJsonObject&);
    void signalLoadPortfolio(const QJsonObject&);

private:
    void prepareReqOperations();
    void prepareReqBondBy();
    void prepareReqShareBy();
    void prepareReqMarket(const QString&);
    void prepareReqLastPrices();

};



#endif // APIREQPAGE_H
