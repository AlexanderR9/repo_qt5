#ifndef BB_APISTRUCT_H
#define BB_APISTRUCT_H

#include <QMap>

//типы страниц пользовательского интерфейса
enum BB_ReqType {rtCandles = 181, rtPositions, rtOrders, rtHistory, rtBag};


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


};

//обобщенная информация о текущем состоянии торгового портфеля
struct BB_BagState
{
    BB_BagState() {reset();}

    float freezed_pos;
    float freezed_order;
    float balance; //total sum on the balance of trading bill
    float pos_result; //current result by all opened positions
    quint16 n_pos;
    quint16 n_order;

    void reset() {freezed_pos = freezed_order = balance = -1; pos_result = 0; n_pos = n_order = 0;}
    float sumFreezed() const;

};

#endif // BB_APISTRUCT_H
