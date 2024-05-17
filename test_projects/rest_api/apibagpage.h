#ifndef APIBAGPAGE_H
#define APIBAGPAGE_H

#include "apipages.h"
#include "instrument.h"

struct TradeOperationData;
struct BagPosition;


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
    void addPosition(const BagPosition&);
    void tryPlaceOrder(TradeOperationData&);
    void prepareOrderData(TradeOperationData&, int max_p = -1);
    void updateCellsColors(int i, const BagPosition&);

private:
    void addGeneralEdit(QString, int&);
    float getCurrentPrice(int) const;
    quint16 paperCount(int) const;

protected slots:
    void slotBagUpdate();
    void slotContextMenu(QPoint);
    void slotBuyOrder();
    void slotSellOrder();
    void slotSetTakeProfit();
    void slotSetStopLoss();

public slots:
    void slotUpdateOrdersInfo(const QMap<QString, QString>&);
    void slotGetPaperCountByTicker(const QString&, int&, float&);
    void slotSetBagStocks(QStringList&);

signals:
    void signalLoadPositions(const QJsonObject&);
    void signalLoadPortfolio(const QJsonObject&);
    void signalGetPaperInfo(QStringList&);
    void signalGetLotSize(const QString&, quint16&);
    void signalSendOrderCommand(const PlaceOrderData&);
    void signalGetBondNominalByUID(const QString&, float&);
    void signalGetBondEndDateByUID(const QString&, QDate&);


};


#endif // APIBAGPAGE_H


