#ifndef APICONFIG_H
#define APICONFIG_H

#include <QStringList>


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


    static QString appDataPath();
    void loadTickers();

};


extern APIConfig api_config;


#endif // APICONFIG_H

