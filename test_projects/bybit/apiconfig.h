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
        QString toStr() const {return QString("CANDLE_SETTINGS: number=%1  size=[%2]").arg(number).arg(size);}
    };

    void reset();

    Candle candle;
    QStringList tickers;
    QStringList favor_tickers;
    QString api_key;
    QString api_key_private;
    quint16 req_delay;
    quint16 req_limit_pos;


    static QString appDataPath();
    static QByteArray calcHMACSha256(QByteArray, QByteArray); //рассчет подписи для идентификации
    static qint64 toTimeStamp(quint16 d, quint16 m, quint16 y); //перевод во временную метку указанной даты в 00:00 (ms UTC from SinceEpoch)
    static QString fromTimeStamp(qint64, QString mask = QString("dd.MM.yyyy (hh:mm)")); //вернет временную метку в строком формате (UTC)
    static QString userDateMask() {return QString("dd.MM.yyyy");}

    void loadTickers();
    void saveFavorTickers();
    void setApiKeys(QString, QString);

};


extern APIConfig api_config;


#endif // APICONFIG_H

