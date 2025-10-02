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

};

// структура для описания одной позиции пула V3
struct DefiPosition
{
    DefiPosition() {reset();}

    int pid;
    QPair<QString, QString> token_addrs; // адреса пары тоненов пула, в котором открыта поза
    QPair<int, int> tick_range; // оригинальная пара значений тиков, в границах которых открыта поза
    int fee; // 100, 500, 3000, 10000
    QString liq;

    DefiPositionState state; // обновляется при запросе к сети

    void reset();
    bool invalid() const;
    bool hasLiquidity() const; // признак того что в позицию внесена ликвидность
    void fromFileLine(const QString&); // загрузить поля позы из строки файла

    // функции для отображения данных в интерфейсе
    QString interfaceFee() const;
    QString interfaceTickRange() const;
    QString interfacePriceRange() const;
    QString interfaceCurrentPrice() const;
    QString interfaceAssetAmounts() const;
    QString interfaceRewards() const;

    //diag func
    QString toStr() const;

};




#endif // DEFIPOSITION_H
