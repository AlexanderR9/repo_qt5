#ifndef CFDCONFIGOBJ_H
#define CFDCONFIGOBJ_H

#include "lsimpleobj.h"

class QDomNode;


//CFDDataSource
struct CFDDataSource
{
    CFDDataSource() {reset();}

    int id;
    QString url;

    void reset() {id=-1; url.clear();}
    bool invalid() const {return (id < 0 || url.length() < 10 || url.left(6) != "https:");}
    QString toStr() const {return QString("CFDDataSource: id=%1  url=[%2]").arg(id).arg(url);}

};


//CFDObj
struct CFDObj
{
    CFDObj() {reset();}

    QString name;
    QString ticker;
    QString country;
    int source_id;
    QString url_text;
    bool is_insta;

    void reset() {source_id=-1; name = ticker = url_text = QString(); is_insta=false;}
    bool invalid() const {return (source_id < 0 || name.isEmpty() || ticker.isEmpty());}
    QString toStr() const {return QString("CFDObj: (%1)  ticker=%2  source_id=%3  url_text=[%4]  insta=%5  county=%6").
                arg(name).arg(ticker).arg(source_id).arg(url_text).arg(is_insta?"yes":"no").arg(country);}
    QString requestUrl(const QString &base_url) const {return QString("%1%2").arg(base_url).arg(url_text.isEmpty()?ticker:url_text);}

};

// GetDivsParams
struct GetDivsParams
{
    GetDivsParams() {reset();}

    quint8 request_interval; //hours, 0 - off algoritm
    QString source_url;
    quint16 show_last;
    quint16 timer_interval; //sec
    double light_div_size;
    double light_price;
    quint8 look_div_days; //days

    void reset()
    {
        source_url.clear();
        request_interval = 0; timer_interval = 120; show_last = 200;
        light_div_size = 0.5; look_div_days=14; light_price = 155;
    }
    QString toStr() const
    {
        return QString("DivsParams: request_interval=(%1 h)/(%2 sec)  light_div=%3  look_days=%4  light_price=%5  history_size=%6  URL: (%7)").
                arg(request_interval).arg(timer_interval).arg(QString::number(light_div_size, 'f', 2)).
                arg(look_div_days).arg(light_price).arg(show_last).arg(source_url);
    }
};

// CalcActionParams
struct CalcActionParams
{
    CalcActionParams() {reset();}

    double min_recalc_size;
    double notice_day_size;
    double notice_week_size;
    double notice_month_size;

    void reset() {min_recalc_size=0.04; notice_day_size=5; notice_week_size=10; notice_month_size=15;}
    QString toStr() {return QString("ACTION PARAMS: min_recalc_size=%1,  notice_sizes: day(%2)  week(%3)  month(%4)").arg(min_recalc_size).arg(notice_day_size).arg(notice_week_size).arg(notice_month_size);}
};


//CFDConfigObject
class CFDConfigObject : public LSimpleObject
{
    Q_OBJECT
public:
    CFDConfigObject(const QString&, QObject*);
    virtual ~CFDConfigObject() {}

    QString name() const {return QString("cfd_config_obj");}

    void tryLoadConfig();
    QStringList getSources() const;
    QStringList getTickers(bool only_insta = false) const;
    QStringList getCFDObjectData(int) const;
    void getNextTicker(QString&);

    inline int cfdCount() const {return m_cfdList.count();}
    inline const CalcActionParams& calcActionParams() const {return m_actParams;}
    inline const GetDivsParams& divParams() const {return m_divParams;}

protected:
    QString m_configFile;
    QList<CFDDataSource> m_sources;
    QList<CFDObj> m_cfdList;
    CalcActionParams m_actParams;
    GetDivsParams m_divParams;
    int m_curCFDIndex; //for request data

    void loadSources(const QDomNode&);
    void loadCFDList(const QDomNode&);
    void loadActParams(const QDomNode&);
    void loadDivParams(const QDomNode&);

    void reset();
    void sendConfigInfo();

private:
    QString sourceByID(int) const;
    bool containsTicker(QString) const;
    double getDoubleAttrValue(const QDomNode&) const;

public slots:
    void slotSetInstaPtr(const QString&, bool&);


};


#endif //CFDCONFIGOBJ_H


