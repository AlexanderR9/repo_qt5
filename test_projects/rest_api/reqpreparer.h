#ifndef REQPREPARER_H
#define REQPREPARER_H

#include "lsimpleobj.h"

class LHttpApiRequester;
struct PlaceOrderData;

//ApiReqPrepare
class ApiReqPreparer : public LSimpleObject
{
    Q_OBJECT
public:
    ApiReqPreparer(LHttpApiRequester*&, QObject *parent = NULL);
    virtual ~ApiReqPreparer() {}

    QString name() const {return QString("requester_preparer_obj");}
    void prepare(QString, const PlaceOrderData*);
    //void prepareOrderReq(QString, const PlaceOrderData*);
    bool invalidReq() const;

    inline void setCycleData(const QStringList &list) {m_cycleData.clear(); m_cycleData.append(list);}
    inline void clearCycleData() {m_cycleData.clear();}
    inline const QStringList& cycleData() const {return m_cycleData;}


protected:
    LHttpApiRequester*&     m_reqObj;
    QStringList             m_cycleData;

    void setMetaPeriod(QString);
    void prepareHeaders(const QString&);

private:
    void prepareReqOperations(const QString&);
    void prepareReqBondBy();
    void prepareReqShareBy();
    void prepareReqMarket(const QString&);
    void prepareReqOrders(const PlaceOrderData*);
    void prepareReqLastPrices();
    void prepareReqCoupons();
    void prepareReqDivs();

signals:
    void signalGetReqParams(QString&, QString&); //try get tocken and base_uri
    void signalGetSelectedBondUID(QString&);
    void signalGetSelectedStockUID(QString&);
    void signalGetSelectedBondUIDList(QStringList&);
    void signalGetSelectedStockUIDList(QStringList&);
    void signalGetPricesDepth(quint16&);
    void signalGetCandleSize(QString&);


};



#endif // REQPREPARER_H


