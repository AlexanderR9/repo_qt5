#ifndef DEFI_CONFIG_H
#define DEFI_CONFIG_H

#include <QList>
#include <QStringList>


// абстрактаная Defi-сущность
struct DefiEntityBase
{
    DefiEntityBase() :name("?"), chain_id(-1) {}

    QString name;
    int chain_id;

};
// параметры любой сети
struct DefiChain : public DefiEntityBase
{
    DefiChain();

    QString title;
    QString coin; //тикер нативной монеты (оплата газ)
    QString icon_file;

    bool invalid() const;
    QString toStr() const;
    QString fullIconPath() const;

};
// параметры любого токена любой сети
struct DefiToken : public DefiEntityBase
{
    DefiToken();

    quint8  decimal;
    QString address;
    QString icon_file;
    bool is_stable;
    float last_price;

    bool invalid() const;
    QString toStr() const;
    QString fullIconPath() const;
    bool isWraped() const;

};
// параметры любого пула V3
struct DefiPoolV3 : public DefiEntityBase
{
    DefiPoolV3();

    quint16 fee; // 100(0.01%) /  500(0.05) / 3000(0.3%) / 10000 (1.0%)
    QString address; //pool address

};
//параметры для общения с API-bybit
struct BybitSettings
{
    BybitSettings();

    QString public_key;
    QString private_key;
    QString api_server;
    quint16 timeout;

    bool invalid() const;
};


//структура хранящая базовые адреса и названия сетей и токенов, а так же пулов
struct DefiConfiguration
{
    DefiConfiguration() {reset();}

    QList<DefiChain> chains;
    QList<DefiToken> tokens;
    QList<DefiPoolV3> pools;
    BybitSettings bb_settings;
    quint16 delayAfterTX; // seconds

    void reset();
    int chainIndexOf(int) const; // возврвщает индекс элемента chains по указанному id или -1
    int getChainID(QString) const; //получить id сети по ее названию, в случае ошибки вернет -1
    QString nativeTokenName(QString) const; //получить имя нативного токена для указанной сети
    float lastPriceByTokenName(QString) const; // получить последнюю цену по тикеру токена

};


//global var
extern DefiConfiguration defi_config;


#endif // DEFI_CONFIG_H


