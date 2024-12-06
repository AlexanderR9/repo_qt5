#ifndef UG_APISTRUCT_H
#define UG_APISTRUCT_H

#include <QMap>

class QJsonObject;

//типы страниц пользовательского интерфейса
enum UG_ReqType {rtJsonView = 131, rtPools, rtTokens, rtDaysData, rtPositions};


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

//Position data
struct UG_PosInfo
{
    UG_PosInfo(QString cn) :chain(cn.trimmed().toUpper()) {reset();}

    QString id; //ID of position
    QString chain; //chain name
    UG_PoolInfo pool; //pool data for this position
    quint32 ts; //creation time
    qint64 liquidity;

    QPair<double, double> deposited;
    //QPair<double, double> collected;
    QPair<double, double> collectedFees;
    QPair<double, double> withdrawn;


    QPair<qint8, qint8> digit; //точность
    QPair<qint32, qint32> tick_range;
    qint32 cur_tick;
    double token0Price; //cur pool price token0 in units token1

    inline bool isClosed() const {return (liquidity == 0);}
    bool isOut() const; //sing out price range


    void reset();
    bool invalid() const;
    void fromJson(const QJsonObject&);
    void toTableRow(QStringList&) const;
    void calcDigit();
    void calcPricesByTicks(double&, double&) const; //расчет диапазона цен токена0 в единицах токена1

    QString toStr() const;
    QString poolParams() const;
    QString priceRange() const;

private:
    double toStablePrice(const double&) const; //приведение цены к стейблам, в приоритете USDT, затем USDC, если в паре таких нет, то вернет то же значение


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
    quint16 decimal;

    void reset();
    bool invalid() const;
    void toTableRow(QStringList&) const;
    QString toFileLine() const;
    void fromFileLine(const QString&);

};




#endif // UG_APISTRUCT_H
