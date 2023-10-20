#ifndef API_PAGES_H
#define API_PAGES_H

#include "lsimplewidget.h"
#include "instrument.h"

class QSettings;
class LListWidgetBox;
class LTreeWidgetBox;
class LHttpApiRequester;
class QComboBox;


enum APIPageType {aptReq = 181, aptBond, aptStock};

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
    void setServerAPI(int, const QString&);
    void checkReply();
    bool replyOk() const;
    void setExpandLevel(int);

protected:
    LListWidgetBox      *m_sourceBox;
    LTreeWidgetBox      *m_replyBox;
    LHttpApiRequester   *m_reqObj;

    void initWidgets();
    void initSources();
    void prepareReq(int);
    void handleReplyData();
    void saveBondsFile();

signals:
    void signalFinished(int);
    void signalGetReqParams(QString&, QString&); //try get tocken and base_uri
    void signalGetSelectedBondUID(QString&);
    void signalGetPricesDepth(quint16&);
    void signalGetCandleSize(QString&);

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

    static QString dataFile();// {return QString("bonds.txt");}

protected:
    QList<BondDesc>         m_data;
    QComboBox               *m_countryFilterControl;
    QComboBox               *m_riskFilterControl;

    void initFilterBox();
    void reloadTableByData();
    void countryFilter(const QString&);
    void riskFilter(const QString&);
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

    static QString dataFile() {return QString("stocks.txt");}

};



#endif //API_PAGES_H


