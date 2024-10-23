#ifndef UG_APISTRUCT_H
#define UG_APISTRUCT_H

#include <QMap>
//#include <QDateTime>

class QJsonObject;

//типы страниц пользовательского интерфейса
enum UG_ReqType {rtJsonView = 131, rtPools, rtTokens};


//необходимые данные для формирования запроса перед отправкой
struct UG_APIReqParams
{
    UG_APIReqParams() {reset();}

    int req_type;
    QString name;
    //bool is_running;
    QString query;

    void reset() {name="?"; req_type=-1; /*is_running=false;*/ query.clear();}
    bool invalid() const {return (req_type<0 || query.trimmed().isEmpty());}

    static QString strReqTypeByType(int, QString = "");
    static QString userDateMask() {return QString("dd.MM.yyyy");}
    static QString userTimeMask() {return QString("hh:mm:ss");}
    static QString userDateTimeMask() {return QString("dd.MM.yyyy  (hh:mm)");}
    //static QString appDataPath();

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



//Token data
struct UG_TokenInfo
{
    UG_TokenInfo() {reset();}

    QString address; //contract address
    QString ticker;
    QString chain;

    void reset();
    bool invalid() const;
    void toTableRow(QStringList&) const;

};




#endif // UG_APISTRUCT_H
