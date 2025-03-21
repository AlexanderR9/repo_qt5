#ifndef UG_APISTRUCT_H
#define UG_APISTRUCT_H

#include <QMap>

class QJsonObject;

//типы страниц пользовательского интерфейса
enum UG_ReqType {rtJsonView = 131, rtPools, rtTokens, rtDaysData, rtPositions, rtPositionsAct, rtEthers};


//необходимые данные для формирования запроса перед отправкой
struct UG_APIReqParams
{
    UG_APIReqParams() {reset();}

    int req_type;
    QString name;
    QString query;

    void reset() {name="?"; req_type=-1; query.clear();}
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
    UG_PoolInfo(const UG_PoolInfo &other) {setData(other);}

    //members
    QString id;
    double tvl;
    QString token0; //ticker of instrument
    QString token1; //ticker of instrument
    QString token0_id;
    QString token1_id;    
    double fee;
    double volume_all;
    quint32 ts; //creation time
    QPair<QString, QString> feeGrowthGlobal;

    qint32 cur_tick; //tick index (current state param)
    QString pX96; //sqrtPrice
    double token0_price; //cur pool price token0 in units token1  (current state param)
    double token1_price; //cur pool price token1 in units token0  (current state param)


    // funcs
    void reset();
    bool invalid() const;
    void fromJson(const QJsonObject&);
    QString toStr() const;
    void toTableRow(QStringList&) const;
    QString toFileLine() const;
    void fromFileLine(const QString&);
    void setData(const UG_PoolInfo&);
    quint32 age() const; //days


};

//Position data
struct UG_PosInfo
{
    UG_PosInfo(QString cn) :chain(cn.trimmed().toUpper()) {reset();}
    UG_PosInfo(const UG_PosInfo &other) {setData(other);}

    ////////////// MEMBERS //////////////
    QString id; //ID of position
    QString chain; //chain name
    quint32 ts; //creation time
    QString liquidity; // BigNum

    UG_PoolInfo pool; //parent pool data for this position

    QPair<double, double> deposited;
    QPair<double, double> collectedFees;
    QPair<double, double> withdrawn;
    QPair<qint8, qint8> digit; //точность
    QPair<qint32, qint32> tick_range;
    QPair<QString, QString> feeGrowthInside;
    QPair<QString, QString> feeGrowthOutsize_lower;
    QPair<QString, QString> feeGrowthOutsize_upper;

    ////////////////////////////////////////


    bool isClosed() const;// {return (liquidity == 0);}
    bool isOut() const; // out price range
    bool liqNull() const;

    void reset();
    bool invalid() const;
    void fromJson(const QJsonObject&);
    void toTableRow(QStringList&) const;
    void toTableRow_act(QStringList&) const;
    void calcDigit();
    void calcPricesByTicks(double&, double&) const; //расчет диапазона цен токена0 в единицах токена1

    void setData(const UG_PosInfo&);

    QString toStr() const;
    QString toStrFeeGrowth() const;
    QString poolParams() const;
    QString priceRange() const;
    QString unclaimedFees() const;
    QString age() const;
    QString curAssets() const;
    double curAsset0() const;
    double curAsset1() const;
    double byStablePrice() const; //вернет цену в стэйблах если в паре пула есть такая монета, иначе -1

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
