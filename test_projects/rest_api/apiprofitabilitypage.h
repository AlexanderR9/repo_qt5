#ifndef APIPROFITABILITYPAGE_H
#define APIPROFITABILITYPAGE_H

#include "apipages.h"
#include "instrument.h"

#include <QStringList>

struct BondDesc;
struct BCoupon;
struct PlaceOrderData;


//APIProfitabilityPage
class APIProfitabilityPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIProfitabilityPage(QWidget*);
    virtual ~APIProfitabilityPage() {}

    QString iconPath() const {return QString(":/icons/images/crc.png");}
    QString caption() const {return QString("Profitability");}

protected:
    int m_tick;
    PlaceOrderData m_buyData;

    void syncTableData(const BondDesc&, const QStringList&, float);

public slots:
    void slotRecalcProfitability(const BondDesc&, float);

protected slots:
    void slotResizeTimer();
    void slotContextMenu(QPoint);
    void slotBuyOrder();

signals:
    void signalGetCouponRec(const QString&, const BCoupon*&);
    void signalBuyOrder(const PlaceOrderData&);

private:
    float getCurrentPrice(int) const;
    float curAccumulatedCoupon(int) const;

};



#endif // APIPROFITABILITYPAGE_H
