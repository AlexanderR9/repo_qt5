#ifndef APICOUPONPAGE_H
#define APICOUPONPAGE_H


#include "apipages.h"
#include "instrument.h"


class LSearchTableWidgetBox;
class QDomNode;


//APICouponPageAbstract
class APICouponPageAbstract : public APITablePageBase
{
    Q_OBJECT
public:
    APICouponPageAbstract(QWidget*);
    virtual ~APICouponPageAbstract() {clearData();}

    virtual void loadData();

protected:
    QList<CouponRecordAbstract*>  m_data;

    virtual QString dataFile() const = 0;
    virtual void loadFigiRecord(const QDomNode&);
    virtual void reloadTableByData();
    virtual void sortByDate();
    virtual void createRecord(CouponRecordAbstract*&, const QString&) = 0;
    virtual void addRowRecord(const CouponRecordAbstract*, const QPair<QString, QString>&, const QString&) = 0;

private:
    void clearData();

public slots:
    virtual void slotFilter(const QStringList&); //list - visible figi

signals:
    void signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);
    void signalGetTickerByFigi(const QString&, QString&);

};

//APICouponPage
class APICouponPage : public APICouponPageAbstract
{
    Q_OBJECT
public:
    APICouponPage(QWidget*);
    virtual ~APICouponPage() {}

    QString iconPath() const {return QString(":/icons/images/b_scale.svg");}
    QString caption() const {return QString("Coupons");}

protected:
    QString dataFile() const;
    void createRecord(CouponRecordAbstract*&, const QString&);
    void addRowRecord(const CouponRecordAbstract*, const QPair<QString, QString>&, const QString&);

public slots:
    void slotGetCouponRec(const QString&, const BCoupon*&);
    void slotGetCouponInfoByTicker(const QString&, QDate&, float&);


};


//APIDivPage
class APIDivPage : public APICouponPageAbstract
{
    Q_OBJECT
public:
    APIDivPage(QWidget*);
    virtual ~APIDivPage() {}

    QString iconPath() const {return QString(":/icons/images/r_scale.svg");}
    QString caption() const {return QString("Div calendar");}
    void getBagContains(); //получить список тикеров, которые есть в портфеле на данный момент

protected:
    QString dataFile() const;
    void createRecord(CouponRecordAbstract*&, const QString&);
    void addRowRecord(const CouponRecordAbstract*, const QPair<QString, QString>&, const QString&);

signals:
    void signalGetBagStocks(QStringList&);

};


#endif // APICOUPONPAGE_H


