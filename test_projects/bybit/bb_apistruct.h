#ifndef BB_APISTRUCT_H
#define BB_APISTRUCT_H

#include <QMap>
#include <QDateTime>

class QJsonObject;

//типы страниц пользовательского интерфейса
enum BB_ReqType {rtCandles = 181, rtPositions, rtSpotAssets, rtSportOrders, rtOrders, rtHistory, rtSpotHistory,
                 rtBag, rtJsonView, rtFundRate, rtPrices, rtShadows};


//необходимые данные для формирования запроса перед отправкой
struct BB_APIReqParams
{
    BB_APIReqParams() {reset();}
    BB_APIReqParams(QString s, int m) :name(s), metod(m) {uri.clear(); params.clear(); req_type=-1;}

    int req_type;
    QString name;
    int metod;
    QString uri;
    QMap<QString, QString> params;

    void reset() {name="?"; uri.clear(); params.clear(); metod=-1; req_type=-1;}
    bool invalid() const;
    QString fullUri() const; //uri?params
    QString paramsLine() const; //params => str_line(key=value&...)
    QString toStr() const;

    static QString strReqTypeByType(int, QString = "");
    static QString userDateMask() {return QString("dd.MM.yyyy");}
    static QString userTimeMask() {return QString("hh:mm:ss");}
    static QString userDateTimeMask() {return QString("dd.MM.yyyy  (hh:mm)");}

};

//обобщенная информация о текущем состоянии торгового портфеля
struct BB_BagState
{
    BB_BagState() {reset();}

    float freezed_pos;
    float freezed_order;
    float balance; //total sum on the balance of trading bill
    float balance_free; //free sum in the unfined wallet
    float pos_result; //current result by all opened positions
    quint16 n_pos;
    quint16 n_order;
    float fund_balance;

    void reset() {freezed_pos = freezed_order = -1; balance = balance_free = fund_balance = 0; pos_result = 0; n_pos = n_order = 0;}
    float sumFreezed() const;
};


//сводная информация по истории операций
struct BB_HistoryState
{
    BB_HistoryState() {reset();}

    quint16 closed_pos;
    quint16 closed_orders;
    quint16 canceled_orders;
    float paid_commission;
    float total_pnl;

    void reset();
};

//history elements
struct BB_HistoryRecordBase
{
    BB_HistoryRecordBase() {reset();}

     // 6 base fields
    QString uid;
    QString ticker;
    QString action; //buy or sell (order),  long or short (pos)
    float lot_size;
    QString type;
    QString status;

    virtual void reset();
    virtual bool invalid() const {return (uid.isEmpty() || ticker.isEmpty() || action.isEmpty() || lot_size < 0);}
    virtual void fromFileLine(const QString&);

    virtual QString toFileLine() const = 0;
    virtual void fromJson(const QJsonObject&) = 0;
    virtual QStringList toTableRowData() const = 0;
    virtual quint8 filedsCount() const = 0;
    virtual void parseSplitedFileLine(const QStringList&) = 0;

protected:
    quint8 precisionByValue(float, quint8 max_prec = 6) const;

};
struct BB_HistoryPos : public BB_HistoryRecordBase
{
    BB_HistoryPos() {reset();}

    // 6 fields else
    QDateTime closed_time;
    quint8 leverage;
    float open_price;
    float closed_price;
    float total_result; //окончательный результат после вычета всех коммисий
    //QString trigger;

    inline float dPrice() const {return (closed_price - open_price);}
    inline bool isLong() const {return (action.toLower() == "long");}
    inline bool isShort() const {return (action.toLower() == "short");}
    static QStringList tableHeaders();

    void reset();
    QString toFileLine() const;
    void fromJson(const QJsonObject&);
    QStringList toTableRowData() const;
    quint8 filedsCount() const {return 11;}
    void parseSplitedFileLine(const QStringList&);
    QString priceInfo() const;
    float paidFee() const;


};
struct BB_HistoryOrder : public BB_HistoryRecordBase
{
    BB_HistoryOrder() {reset();}

    QDateTime create_time;
    float price;
    bool is_leverage;

    static QStringList tableHeaders();

    inline bool isSell() const {return (action.toLower() == "sell");}
    inline bool isBuy() const {return (action.toLower() == "buy");}
    inline bool isLimit() const {return (type.toLower() == "limit");}
    inline bool isStop() const  {return (type.toLower() == "takeprofit" || type.toLower() == "stoploss");}
    inline bool isCancelled() const {return (status.toLower() == "cancelled");}

    void reset();
    QString toFileLine() const;
    quint8 filedsCount() const {return 9;}
    void parseSplitedFileLine(const QStringList&);
    QStringList toTableRowData() const;
    void fromJson(const QJsonObject&);

};
struct BB_HistorySpot : public BB_HistoryRecordBase
{
    BB_HistorySpot() {reset();}

    QDateTime triger_time;
    float price;
    float fee; //абсолютное значение
    QString fee_ticker;


    inline bool isSell() const {return (action.toLower() == "sell");}
    inline bool isBuy() const {return (action.toLower() == "buy");}

    float pFee() const; //коммисия в процентах
    float usdSize() const; //deal size for USDT
    QString resultDeal() const;

    void reset();
    QString toFileLine() const;
    quint8 filedsCount() const {return 10;}
    void parseSplitedFileLine(const QStringList&);
    QStringList toTableRowData() const;
    void fromJson(const QJsonObject&);

    static QStringList tableHeaders();
};

//контейнер для хранения всех цен по всем избранным инструментам.
//добавление идет не раньше чем через час.
struct BB_PricesContainer
{
    BB_PricesContainer() {reset();}

    struct PricePoint
    {
        PricePoint() {reset();}
        PricePoint(const QString &s, float p, const qint64 &t) :ticker(s), price(p), time(t) {}

        QString ticker;
        float price;
        qint64 time;

        void reset() {ticker.clear(); price = 0; time = -1;}
        QString toStr() const {return QString("PricePoint (%1):  price[%2]  time[%3]").arg(ticker).arg(price).arg(time);}
        QString toFileElement() const {return QString("[%1 / %2 / %3]").arg(ticker).arg(QString::number(price, 'f', 5)).arg(time);}

    };

    QList<PricePoint> data;
    inline quint32 size() const {return data.count();}

    static QString priceFile() {return QString("prices.txt");} //file data
    static quint32 minIntervalPrices() {return 3600;} //secs, min interval between neighbors points

    void reset() {data.clear();}
    void reloadPrices(const QString&); //load container pirces from file_data
    void addPricePoint(const QString&, const float&, const qint64&); //try add price for ticker
    int lastPriceByTicker(const QString&) const; //индекс самой свежей цены по указанному тикеру, или -1
    void toFileData(QString&);

    //получить отклонение цены от указанной в %, за указанное количество дней.
    //в случае отсутствия необходимых данных для расчет этой величины вернет -9999.
    //параметры: ticker, cur_price, cur_time, days_count.
    float getDeviation(const QString&, float, const qint64&, quint16) const;

    //вернет цену ближайшую к указанной дате, но не позднее ее.
    //в случае отсутствия необходимых данных вернет -1.
    //параметры: ticker, datetime
    float getNearPriceByDate(const QString&, qint64) const;


private:
    void parsePricePoint(QString);

};


#endif // BB_APISTRUCT_H
