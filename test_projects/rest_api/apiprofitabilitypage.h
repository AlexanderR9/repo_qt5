#ifndef APIPROFITABILITYPAGE_H
#define APIPROFITABILITYPAGE_H

#include "apipages.h"
#include "instrument.h"
#include "ui_assetinfowidget.h"
#include "lsimpledialog.h"

#include <QStringList>

struct BondDesc;
struct BCoupon;
struct PlaceOrderData;
class QTableWidgetItem;
struct AssetFavorRecord;


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

    void syncTableData(const BondDesc&, const QStringList&, float, float);
    int indexOf(const QString&) const; //find record in the table by figi, return row_index or -1
    void sortByProfitability(int, float);
    void initAssetInfoBox();

public slots:
    void slotRecalcProfitability(const BondDesc&, float);

protected slots:
    void slotResizeTimer();
    void slotContextMenu(QPoint);
    void slotBuyOrder();
    void slotTableClicked(QTableWidgetItem*);
    void slotGetCurrentPriceBySelected(float&);
    void slotAddFavor();
    void slotRemoveFavor();

signals:
    void signalGetCouponRec(const QString&, const BCoupon*&);
    void signalBuyOrder(const PlaceOrderData&);
    void signalUpdateInfoBox(const QString&);

    //translation by AssetInfoWidget
    void signalGetBondRiskByTicker(const QString&, QString&);
    void signalGetPaperCountByTicker(const QString&, int&, float&);
    void signalGetCouponInfoByTicker(const QString&, QDate&, float&);
    void signalGetEventsHistoryByTicker(const QString&, QStringList&);

    //for APICandlesPage
    void signalRemoveFavorAsset(const QString&);
    void signalAddFavorAsset(const AssetFavorRecord&);

private:
    float getCurrentPrice(int) const;
    float curAccumulatedCoupon(int) const;

};


//AssetInfoWidget
class AssetInfoWidget : public QWidget, public Ui::AssetInfoWidget
{
    Q_OBJECT
public:
    AssetInfoWidget(QWidget *parent = 0);
    virtual ~AssetInfoWidget() {}

public slots:
    void slotReset();
    void slotRunUpdate(const QString&);

protected:
    void init();
    void refreshTable();
    void updateRisk(QString);
    void updateBag(int, float);
    void updateCoupon(const QDate&, float);
    void updateHistory(const QStringList&);
    void updatePrice(float);

signals:
    void signalGetBondRiskByTicker(const QString&, QString&);
    void signalGetPaperCountByTicker(const QString&, int&, float&);
    void signalGetCouponInfoByTicker(const QString&, QDate&, float&);
    void signalGetEventsHistoryByTicker(const QString&, QStringList&);
    void signalGetCurrentPriceBySelected(float&);

};

// AssetFavorDialog
class AssetFavorDialog : public LSimpleDialog
{
    Q_OBJECT
public:
    AssetFavorDialog(QString, AssetFavorRecord&, QWidget*);
    virtual ~AssetFavorDialog() {}

protected:
    AssetFavorRecord &m_record;

    void init();
    void updateTitle();
    void setFieldsByRec();

private:
    bool isAddFavor() const;

protected slots:
    void slotApply();


};


#endif // APIPROFITABILITYPAGE_H
