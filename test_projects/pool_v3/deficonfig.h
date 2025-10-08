#ifndef DEFI_CONFIG_H
#define DEFI_CONFIG_H

#include <QList>
#include <QStringList>

struct DefiPosition;


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

    quint16 fee; // 100(0.01%) /  500(0.05%) / 3000(0.3%) / 10000 (1.0%)
    QString address; //pool address
    QString token0_addr;
    QString token1_addr;
    bool is_stable; // признак того что пул состоит из обоих стейблов

    // поле name для этой структуры заполняется как пара тикеров token0/token1 (пример WPOL/USDC)

    float floatFee() const {return fee/float(10000);}
    QString strFloatFee() const {return QString("%1%").arg(QString::number(floatFee(), 'f', 2));}
    bool invalid() const;
    QString toStr() const;

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
//информация о приоритете токенов из пары пула
struct PoolTokenPrioritet
{
    PoolTokenPrioritet();
    QString pair; // пара токенов вида ETH/USDT, порядок не важен
    QString price_token; // токен из пары для которого отображать цену в единицах второго
    QString desired_token; // токен в котором исчислять вложения/доход/убыток (короче вести стату).

    bool invalid() const;
    QString toStr() const;
};


//структура хранящая базовые адреса и названия сетей и токенов, а так же пулов
struct DefiConfiguration
{
    DefiConfiguration() {reset();}

    QList<DefiChain> chains;
    QList<DefiToken> tokens;
    QList<DefiPoolV3> pools;
    QList<PoolTokenPrioritet> prioritet_data;

    BybitSettings bb_settings;
    quint16 delayAfterTX; // seconds
    QString target_wallet;

    void reset();
    int chainIndexOf(int) const; // возврвщает индекс элемента chains по указанному id или -1
    int getChainID(QString) const; //получить id сети по ее названию, в случае ошибки вернет -1
    QString nativeTokenName(QString) const; //получить имя нативного токена для указанной сети
    float lastPriceByTokenName(QString) const; // получить последнюю цену по тикеру токена
    void findPoolTokenAddresses(DefiPoolV3&); //найти и обновить адреса пары токенов для указанного пула
    int getPoolTokenPriceIndex(QString) const; //найти в prioritet_data указанную пару и выдать индекс токена из пары для которого отображать цену
    int getPoolTokenAmountIndex(QString) const; //найти в prioritet_data указанную пару и выдать индекс токена из пары для которого отображать количество
    int getPoolIndex(QString) const; // найти в контейнере pools пул по его адресу и выдать индекс
    int getPoolIndexByPosition(const DefiPosition&) const; // найти в контейнере pools пул по объекту позиции
    int getTokenIndex(QString, int) const; // найти в контейнере tokens asset по его адресу и ID_chain и выдать индекс
    QString tokenNameByAddress(QString, int) const; // найти в контейнере tokens asset по его адресу и ID_chain и выдать его название(тикер)

};


//global var
extern DefiConfiguration defi_config;


#endif // DEFI_CONFIG_H


