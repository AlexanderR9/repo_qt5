#ifndef REQPREPARER_H
#define REQPREPARER_H

#include "lsimpleobj.h"

class LHttpApiRequester;



//ApiReqPrepare
class ApiReqPreparer : public LSimpleObject
{
    Q_OBJECT
public:
    ApiReqPreparer(LHttpApiRequester*&, QObject *parent = NULL);
    virtual ~ApiReqPreparer() {}

    QString name() const {return QString("requester_preparer_obj");}
    void prepare(QString);
    bool invalidReq() const;

protected:
    LHttpApiRequester*&     m_reqObj;


private:
    void prepareReqOperations();
    void prepareReqBondBy();
    void prepareReqShareBy();
    void prepareReqMarket(const QString&);
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


