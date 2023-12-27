#ifndef BB_APISTRUCT_H
#define BB_APISTRUCT_H

#include <QMap>

enum BB_ReqType {rtCandles = 181, rtPositions, rtOrders, rtHistory};


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


};


#endif // BB_APISTRUCT_H
