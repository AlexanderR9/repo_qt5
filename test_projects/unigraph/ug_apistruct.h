#ifndef UG_APISTRUCT_H
#define UG_APISTRUCT_H

#include <QMap>

class QJsonObject;

//типы страниц пользовательского интерфейса
enum UG_ReqType {rtJsonView = 131, rtPools, rtTokens, rtDaysData};


//необходимые данные для формирования запроса перед отправкой
struct UG_APIReqParams
{
    UG_APIReqParams() {reset();}

    int req_type;
    QString name;
    QString query;

    void reset() {name="?"; req_type=-1; /*is_running=false;*/ query.clear();}
    bool invalid() const {return (req_type<0 || query.trimmed().isEmpty());}

    static QString strReqTypeByType(int, QString = "");
    static QString userDateMask() {return QString("dd.MM.yyyy");}
    static QString userTimeMask() {return QString("hh:mm:ss");}
    static QString userDateTimeMask() {return QString("dd.MM.yyyy  (hh:mm)");}

};


//Pool data
struct UG_PoolInfo
{
    UG_PoolInfo() {reset();}

    QString id;
    double tvl;
    QString token0;
    QString token1;
    QString token0_id;
    QString token1_id;
    double fee;
    double volume_all;
    quint32 ts; //creation time


    void reset();
    bool invalid() const;
    void fromJson(const QJsonObject&);
    QString toStr() const;
    void toTableRow(QStringList&) const;
    QString toFileLine() const;
    void fromFileLine(const QString&);

};

//Pool day data
struct UG_PoolDayData
{
    UG_PoolDayData() {reset();}

    quint32 date;
    double tvl;
    double feesSize;
    double volume;
    double price;

    void reset();
    bool invalid() const;
    void fromJson(const QJsonObject&);
    void toTableRow(QStringList&) const;
    QString toStr() const;

};


//Token data
struct UG_TokenInfo
{
    UG_TokenInfo() {reset();}
    UG_TokenInfo(QString a, QString t, QString c);

    QString address; //contract address
    QString ticker;
    QString chain;
    QString name; //full name
    double collected_fees; //usd
    double tvl; //usd
    double total_supply; //token units

    void reset();
    bool invalid() const;
    void toTableRow(QStringList&) const;
    QString toFileLine() const;
    void fromFileLine(const QString&);

};




#endif // UG_APISTRUCT_H
