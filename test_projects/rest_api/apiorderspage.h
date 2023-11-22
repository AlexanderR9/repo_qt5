#ifndef APIORDERSPAGE_H
#define APIORDERSPAGE_H


#include "apipages.h"
#include "instrument.h"

class QJsonObject;


//APIOrdersPage
class APIOrdersPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIOrdersPage(QWidget*);
    virtual ~APIOrdersPage() {}

    QString iconPath() const {return QString(":/icons/images/clock.svg");}
    QString caption() const {return QString("Orders");}

protected:
    QList<OrderData>    m_orders;

    void reloadTableByData();
    void addRowRecord(const OrderData&, const QPair<QString, QString>&);

public slots:
    void slotLoadOrders(const QJsonObject&);

signals:
    void signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);
    void signalGetTickerByFigi(const QString&, QString&);
    void signalGetPaperTypeByUID(const QString&, QString&);


};




#endif // APIORDERSPAGE_H


