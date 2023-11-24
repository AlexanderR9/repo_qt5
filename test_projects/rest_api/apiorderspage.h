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
    virtual ~APIOrdersPage() {clearData();}

    QString iconPath() const {return QString(":/icons/images/clock.svg");}
    QString caption() const {return QString("Orders");}

protected:
    QList<OrderData*>    m_orders;

    void updateTableByData(bool);
    void addRowRecord(const OrderData*, const QPair<QString, QString>&);
    void clearData();
    void clearDataByKind(bool);

public slots:
    void slotLoadOrders(const QJsonObject&);
    void slotLoadStopOrders(const QJsonObject&);

protected slots:
    void slotContextMenu(QPoint);
    void slotCancelOrder();

signals:
    void signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);
    void signalGetTickerByFigi(const QString&, QString&);
    void signalGetPaperTypeByUID(const QString&, QString&);
    void signalGetBondNominalByUID(const QString&, float&);
    void signalCancelOrder(const QStringList&);

};




#endif // APIORDERSPAGE_H


