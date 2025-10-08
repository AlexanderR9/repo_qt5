#ifndef DEFIPOSITION_H
#define DEFIPOSITION_H

#include <QDateTime>
#include <QPair>


class QJsonObject;

// структура для хранения полей текущего состояния позиции
struct DefiPositionState
{
    DefiPositionState() {reset();}

    float price0;
    QPair<float, float> price0_range;
    int pool_tick;
    QPair<float, float> asset_amounts;
    QPair<float, float> rewards;

    void reset();
    void update(const QJsonObject&);
    bool invalid() const;
    float price1() const; // цена токена_1 в единицах 0-го

};

// структура для описания одной позиции пула V3
struct DefiPosition
{
    DefiPosition() {reset();}

    int pid;
    QPair<QString, QString> token_addrs; // адреса пары тоненов пула, в котором открыта поза
    QPair<QString, QString> token_names; // тикеры пары тоненов пула, в котором открыта поза
    QPair<int, int> tick_range; // оригинальная пара значений тиков, в границах которых открыта поза
    int fee; // 100, 500, 3000, 10000
    QString liq;

    // дополнительные расчетные параметры пула этой позиции
    quint8 tprice_index_desired; // индекс целевого токена в паре, 0/1 (для привычного отображения цены)
    quint8 tamount_index_desired; // индекс целевого токена в паре, 0/1 (для привычного отображения сумм токенов)
    bool stable_pool; // пул этой позы состоит из обоих стейблов


    // текущее состояние позы, обновляется при запросе к сети
    DefiPositionState state;

    void reset();
    bool invalid() const;
    bool hasLiquidity() const; // признак того что в позицию внесена ликвидность
    void fromFileLine(const QString&); // загрузить поля позы из строки файла
    void calcTIndex(QString); // определить переменные t*_index_desired

    // расчетные функции
    float lockedDesiredSum() const; // количество текущих вложенных активов в единицах целевого токена из пары
    float lockedSum() const; // количество текущих вложенных активов в единицах стейблов
    float rewardDesiredSum() const; // количество текущих невостребованных наград в единицах целевого токена из пары
    float rewardSum() const; // количество текущих вложенных активов в единицах стейблов


    bool isOutRange() const; // признак что позиция (с ликвидностью) вышла из диапазона

    // функции для отображения данных в интерфейсе
    QString interfaceFee() const;
    QString interfaceTickRange() const;
    QString interfacePriceRange() const;
    QString interfaceCurrentPrice() const;
    QString interfaceAssetAmounts() const;
    QString interfaceRewards() const;
    QString interfaceAssetAmountsDesired() const;
    QString interfaceRewardsDesired() const;

    //diag func
    QString toStr() const;


};




#endif // DEFIPOSITION_H
