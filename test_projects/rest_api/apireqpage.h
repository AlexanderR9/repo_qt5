#ifndef APIREQPAGE_H
#define APIREQPAGE_H

#include "lsimplewidget.h"

class LHttpApiRequester;
class ApiReqPreparer;
class CycleWorker;



//APIReqPage
class APIReqPage : public LSimpleWidget
{
    Q_OBJECT
public:
    enum ReqUpdateInfo {ruiOrders = 110, ruiStopOrders, ruiBagPositions, ruiBagAmount, ruiNone = 0};

    APIReqPage(QWidget*);
    virtual ~APIReqPage();

    QString iconPath() const {return QString(":/icons/images/b_scale.svg");}
    QString caption() const {return QString("API request");}
    virtual void resetPage();

    void trySendReq();
    void autoStartReq(QString);
    void setServerAPI(int, const QString&);
    void checkReply();
    bool replyOk() const;
    void setExpandLevel(int);
    bool requesterBuzy() const;
    void updateOrders();
    void updateBag();
    void updateEvents();

    inline void setPrintHeaders(bool b) {m_printHeaders = b;}


protected:
    LListWidgetBox      *m_sourceBox;
    LTreeWidgetBox      *m_replyBox;
    LHttpApiRequester   *m_reqObj;
    ApiReqPreparer      *m_reqPreparer;
    bool                 m_printHeaders;
    CycleWorker         *m_cycleWroker;
    int                  m_needUpdateInfo;

    void initWidgets();
    void initSources();
    void initReqObject();
    void initReqPreparer();
    void initCycleWorker();
    void handleReplyData();
    void saveBondsFile();
    void saveStocksFile();
    void parseUserID();
    void printHeaders(QString s = "req"); //param - req or resp
    void prepareCycleData();
    void needUpdateInfo();

    inline bool isNeedUpdateInfo() const {return (m_needUpdateInfo != ruiNone);}

private:
    void standardRequest(const QString&, const QStringList &req_data = QStringList());
    void toDebugReqMetadata(); //diag func
    bool selectSrcRow(QString) const;

protected slots:
    void slotCycleWorkerFinished();
    void slotCycleWorkerNextReq();
    void slotReqFinished(int);

public slots:
    void slotTrySendOrderReq(const QStringList&);

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
    void signalLoadEvents(const QJsonObject&);
    void signalLoadOrders(const QJsonObject&);
    void signalLoadStopOrders(const QJsonObject&);
    void signalGetCycleData(QStringList&);
    void signalGetBondCycleData(QStringList&);
    void signalGetStockCycleData(QStringList&);
    void signalCyclePrice(const QString&, float);
    void signalDisableActions(bool);

};



#endif // APIREQPAGE_H
