#ifndef APICONFIG_H
#define APICONFIG_H

#include <QStringList>
#include <QByteArray>


//APIConfig
struct APIConfig
{
    APIConfig() {reset();}

    struct Candle
    {
        Candle() {reset();}
        int number;
        QString size;
        void reset() {number = 100; size="D";}
    };

    void reset();

    Candle candle;
    QStringList tickers;
    QString api_key;
    QString api_key_private;
    quint16 req_delay;
    quint16 req_limit_pos;


    static QString appDataPath();
    static QByteArray calcHMACSha256(QByteArray, QByteArray); //рассчет подписи для идентификации

    void loadTickers();
    void setApiKeys(QString, QString);

};


extern APIConfig api_config;


#endif // APICONFIG_H

