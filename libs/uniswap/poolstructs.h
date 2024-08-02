#ifndef POOLSTRUCTS_H
#define POOLSTRUCTS_H

#include <QMap>



//pools fee kind
//от размера комиcсии зависит размер одной ценовой корзины в тиках.
enum PoolFeeSize {pfs001 = 1201, pfs005, pfs025, pfs03, pfs1};


//input params for calculation
struct InputPoolParams
{
    InputPoolParams() {reset();}

    int fee_type; //element of PoolFeeSize
    QPair<double, double> range; //prices range
    double cur_price;
    quint8 input_token; // 0/1 (token0 or token1)
    double input_size; // for input_token
    bool validity; //признак того что все входные параметры заданы корректно

    void reset();
    void setData(const InputPoolParams&);
    bool curPriceOutRange() const;
    double dRange() const {return (range.second - range.first);}
    void normalizeCurPrice(); //если cur_price больше 0 и одновременно выходит из диапазона то ее значение станет равным одной из границ диапазона
    bool curPriceIsBound() const; //признак того что текущая цена лежит на одно из границ диапазона

    QString toStr() const; //diag func

};

//out params for calculation
struct PoolParamsCalculated
{
    PoolParamsCalculated() {reset();}

    double token0_size; //volatility asset, an example: ARB
    double token1_size; //stable asset
    double L; //user added liquidity

    QPair<qint64, qint64> tick_range; //номера тиков границ ценового диапазона заданного пользователем
    QMap<int, double> range_prices; //all bin_range prices, key - begin bin tick, value-price

    void reset();
    void fillPrices(int);
    double assetsSum(const double&) const; //сумма вложеных активов выраженных в единицах token1, при указанной цене
};

//gui result after calculation
struct GuiPoolResults
{
    GuiPoolResults() {reset();}

    double cur_price;
    double L; //user added liquidity

    QPair<double, double> token_sizes; //prices range
    QPair<double, double> price_range; //prices range
    QPair<qint64, qint64> tick_range; //номера тиков границ ценового диапазона заданного пользователем

    void reset();

};


#endif // POOLSTRUCTS_H

