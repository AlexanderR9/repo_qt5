#ifndef API_PAGES_H
#define API_PAGES_H

#include "lsimplewidget.h"
#include "instrument.h"

class QSettings;
class QComboBox;
class BagState;

enum APIPageType {aptReq = 181, aptBond, aptStock, aptCoupon, aptDiv, aptProfitability,
                    aptEvent, aptOrders, aptBag};


//APITablePageBase
class APITablePageBase : public LSimpleWidget
{
    Q_OBJECT
public:
    APITablePageBase(QWidget*);
    virtual ~APITablePageBase() {}

    virtual void resetPage();

protected:
    LSearchTableWidgetBox   *m_tableBox;
    QGroupBox               *m_filterBox;

    void initWidgets();
    virtual void countryFilter(const QString&);
    virtual void setSelectedUID(QString&, quint16) {}
    virtual void setCycleItem(QString&, quint16) {}
    virtual int priceCol() const;
    virtual QString figiByRecNumber(quint16) const {return QString();}
    virtual void calcProfitability(const QString&, float) {}


public slots:
    virtual void slotGetPaperInfo(QStringList&) {}
    virtual void slotSetSelectedUID(QString&);
    virtual void slotSetSelectedUIDList(QStringList&);
    virtual void slotSetCycleData(QStringList&);
    virtual void slotCyclePrice(const QString&, float);


};


//APIBondsPage
class APIBondsPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIBondsPage(QWidget*);
    virtual ~APIBondsPage() {}

    QString iconPath() const {return QString(":/icons/images/bond.png");}
    QString caption() const {return QString("Bonds list");}

    virtual void resetPage();
    void loadData();

    static QString dataFile();

protected:
    QList<BondDesc>          m_data;
    QComboBox               *m_countryFilterControl;
    QComboBox               *m_riskFilterControl;
    QComboBox               *m_finishDateControl;

    void initFilterBox();
    void reloadTableByData();
    void riskFilter(const QString&);
    void dateFilter(const QString&);
    void sortByDate();
    void setSelectedUID(QString&, quint16);
    void setCycleItem(QString&, quint16);
    QString figiByRecNumber(quint16) const;
    void calcProfitability(const QString&, float);

protected slots:
    void slotFilter(QString);

public slots:
    void slotGetPaperInfo(QStringList&);
    void slotGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);
    void slotGetTickerByFigi(const QString&, QString&);
    void slotGetPaperTypeByUID(const QString&, QString&);
    void slotGetBondNominalByUID(const QString&, float&);
    void slotGetBondEndDateByUID(const QString&, QDate&);

signals:
    void signalFilter(const QStringList&); //list - visible figi
    void signalNeedCalcProfitability(const BondDesc&, float);

};


//APIStocksPage
class APIStocksPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIStocksPage(QWidget*);
    virtual ~APIStocksPage() {}

    QString iconPath() const {return QString(":/icons/images/stock.png");}
    QString caption() const {return QString("Stocks list");}

    virtual void resetPage();
    void loadData();

    static QString dataFile();

protected:
    QList<StockDesc>         m_data;
    QComboBox               *m_countryFilterControl;
    QComboBox               *m_currencyFilterControl;

    void initFilterBox();
    void currencyFilter(const QString&);
    void reloadTableByData();
    void setSelectedUID(QString&, quint16);
    void setCycleItem(QString&, quint16);
    QString figiByRecNumber(quint16) const;

protected slots:
    void slotFilter(QString);

public slots:
    void slotGetPaperInfo(QStringList&);
    void slotGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);
    void slotGetTickerByFigi(const QString&, QString&);
    void slotGetPaperTypeByUID(const QString&, QString&);
    void slotGetLotSize(const QString&, quint16&);

signals:
    void signalFilter(const QStringList&); //list - visible figi

};




#endif //API_PAGES_H


