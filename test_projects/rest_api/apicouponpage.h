#ifndef APICOUPONPAGE_H
#define APICOUPONPAGE_H


#include "apipages.h"
#include "instrument.h"


class LSearchTableWidgetBox;
class QDomNode;


//APICouponPage
class APICouponPage : public APITablePageBase
{
    Q_OBJECT
public:
    APICouponPage(QWidget*);
    virtual ~APICouponPage() {}

    //virtual void resetPage() {}
    void loadData();

    QString iconPath() const {return QString(":/icons/images/b_scale.svg");}
    QString caption() const {return QString("Coupons");}

protected:
    QList<BCoupon>  m_data;

    void loadFigiCoupons(const QDomNode&);
    void reloadTableByData();

signals:
    void signalGetPaperInfoByFigi(const QString&, QPair<QString, QString>&);

};



//APIDivPage
class APIDivPage : public APITablePageBase
{
    Q_OBJECT
public:
    APIDivPage(QWidget*);
    virtual ~APIDivPage() {}

    virtual void resetPage() {}

    QString iconPath() const {return QString(":/icons/images/r_scale.svg");}
    QString caption() const {return QString("Div calendar");}

protected:

};


#endif // APICOUPONPAGE_H


