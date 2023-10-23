#ifndef API_PAGES_H
#define API_PAGES_H

#include "lsimplewidget.h"
#include "instrument.h"

class QSettings;
class LListWidgetBox;
class LTreeWidgetBox;
class LHttpApiRequester;
class QComboBox;
class BagState;

enum APIPageType {aptReq = 181, aptBond, aptStock, aptBag};

//APIReqPage
class APIReqPage : public LSimpleWidget
{
    Q_OBJECT
public:
    APIReqPage(QWidget*);
    virtual ~APIReqPage() {}

    QString iconPath() const {return QString(":/icons/images/b_scale.svg");}
    QString caption() const {return QString("API request");}
    virtual void resetPage();
    void trySendReq();
    void autoStartReq(QString);
    void setServerAPI(int, const QString&);
    void checkReply();
    bool replyOk() const;
    void setExpandLevel(int);
    inline void setPrintHeaders(bool b) {m_printHeaders = b;}
    bool requesterBuzy() const;

protected:
    LListWidgetBox      *m_sourceBox;
    LTreeWidgetBox      *m_replyBox;
    LHttpApiRequester   *m_reqObj;
    bool                 m_printHeaders;

    void initWidgets();
    void initSources();
    void prepareReq(int);
    void handleReplyData();
    void saveBondsFile();
    void saveStocksFile();
    void parseUserID();

signals:
    void signalFinished(int);
    void signalGetReqParams(QString&, QString&); //try get tocken and base_uri
    void signalGetSelectedBondUID(QString&);
    void signalGetPricesDepth(quint16&);
    void signalGetCandleSize(QString&);
    void signalLoadPositions(const QJsonObject&);
    void signalLoadPortfolio(const QJsonObject&);

};


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
    QList<BondDesc>         m_data;
    QComboBox               *m_countryFilterControl;
    QComboBox               *m_riskFilterControl;
    QComboBox               *m_finishDateControl;

    void initFilterBox();
    void reloadTableByData();
    void riskFilter(const QString&);
    void dateFilter(const QString&);
    void sortByDate();

protected slots:
    void slotFilter(QString);

public slots:
    void slotSetSelectedBondUID(QString&);

};


//APIStoksPage
class APIStoksPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIStoksPage(QWidget*);
    virtual ~APIStoksPage() {}

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

protected slots:
    void slotFilter(QString);

};


//APIBagPage
class APIBagPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIBagPage(QWidget*);
    virtual ~APIBagPage() {}

    QString iconPath() const {return QString(":/icons/images/bag.svg");}
    QString caption() const {return QString("Bag");}

    //inline const BagState* bagObj() const {return m_bag;}
    //virtual void resetPage();
protected:
    BagState    *m_bag;

    void initFilterBox();

private:
    void addGeneralEdit(QString, int);

signals:
    void signalLoadPositions(const QJsonObject&);
    void signalLoadPortfolio(const QJsonObject&);

protected slots:
    void slotBagUpdate();

};



#endif //API_PAGES_H


