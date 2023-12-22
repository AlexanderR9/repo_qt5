#ifndef JSON_VIEWPAGE_H
#define JSON_VIEWPAGE_H

#include "lsimplewidget.h"

/*
class LHttpApiRequester;
class ApiReqPreparer;
class CycleWorker;
struct PlaceOrderData;
*/

class QJsonObject;

//JSONViewPage
class JSONViewPage : public LSimpleWidget
{
    Q_OBJECT
public:
    //enum ReqUpdateInfo {ruiOrders = 110, ruiStopOrders, ruiBagPositions, ruiBagAmount, ruiNone = 0};

    JSONViewPage(QWidget*);
    virtual ~JSONViewPage() {}

    QString iconPath() const {return QString(":/icons/images/tree.png");}
    QString caption() const {return QString("JSON viewer");}
    void setExpandLevel(int);
    //virtual void resetPage();

    /*
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
    void getVisibleBondPrices();

    inline void setPrintHeaders(bool b) {m_printHeaders = b;}
*/

protected:
    LTreeWidgetBox      *m_replyBox;

    void initWidgets();

    /*
    LListWidgetBox      *m_sourceBox;
    LHttpApiRequester   *m_reqObj;
    ApiReqPreparer      *m_reqPreparer;
    bool                 m_printHeaders;
    CycleWorker         *m_cycleWroker;
    int                  m_needUpdateInfo;

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
    void standardRequest(const QString&, const PlaceOrderData *pod = NULL);
    void toDebugReqMetadata(); //diag func
    bool selectSrcRow(QString) const;

protected slots:
    void slotCycleWorkerFinished();
    void slotCycleWorkerNextReq();
    void slotReqFinished(int);
*/
public slots:
    void slotReloadJsonReply(int, const QJsonObject&);

  //  void slotTrySendOrderReq(const PlaceOrderData&); //попытка выполнить операцию c ордером: buy/sell/cancel
/*
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
    */

};



#endif // JSON_VIEWPAGE_H
