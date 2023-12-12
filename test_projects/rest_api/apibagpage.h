#ifndef APIBAGPAGE_H
#define APIBAGPAGE_H

#include "apipages.h"
#include "instrument.h"


//APIBagPage
class APIBagPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIBagPage(QWidget*);
    virtual ~APIBagPage() {}

    QString iconPath() const {return QString(":/icons/images/bag.svg");}
    QString caption() const {return QString("Bag");}

protected:
    BagState    *m_bag;
    PlaceOrderData m_orderData;

    void initFilterBox();
    void reloadPosTable();

private:
    void addGeneralEdit(QString, int&);
    float getCurrentPrice(int) const;

signals:
    void signalLoadPositions(const QJsonObject&);
    void signalLoadPortfolio(const QJsonObject&);
    void signalGetPaperInfo(QStringList&);
    void signalGetLotSize(const QString&, quint16&);
    void signalSendOrderCommand(const PlaceOrderData&);

protected slots:
    void slotBagUpdate();
    void slotContextMenu(QPoint);
    void slotBuyOrder();
    void slotSellOrder();
    void slotSellStopOrder();

};


#endif // APIBAGPAGE_H


