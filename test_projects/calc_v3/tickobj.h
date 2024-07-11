#ifndef TICKOBJ_H
#define TICKOBJ_H

#include "lsimpleobj.h"

enum TickSpacerType {tst001 = 0, tst005, tst03, tst1};

#include <QMap>


//input params for calculation
struct PoolParamsStruct
{
    PoolParamsStruct() {reset();}

    int fee_type;
    QPair<double, double> range; //prices range
    double cur_price;
    quint8 input_token; // token0 or token1
    double input_size; // for input_token
    bool validity;

    void reset();
    void setData(const PoolParamsStruct&);
    QString toStr() const;
    bool curPriceOutRange() const;
    double dRange() const {return (range.second - range.first);}

};

//out params for calculation
struct PoolParamsCalculated
{
    PoolParamsCalculated() {reset();}

    double token0_size; //volatility asset, an example: ARB
    double token1_size; //stable asset
    QPair<int, int> tick_range;
    quint16 bin_count;
    int cur_bin; //bin where cur_price or -1 if cur_price is out the range
    QMap<int, double> range_prices; //all bin_range prices, key - begin bin tick, value-price

    void reset();
    void fillPrices(int);
    void findCurrentBin(const double&);
    void outBinPrices();


};

//tick calculator
class TickObj : public LSimpleObject
{
    Q_OBJECT
public:
    TickObj(QObject *parent = NULL);
    virtual ~TickObj() {}

    virtual QString name() const {return QString("tick_obj");}

    static quint16 binSize(int); //интервал между тиками (размер корзины) по типу размера комиссии пула
    static QString captionByFeeType(int);
    static double basePips() {return 1.0001;}

    inline void setParams(const PoolParamsStruct &p) {m_params.setData(p);}

    void recalc();

protected:
    PoolParamsStruct m_params;
    PoolParamsCalculated m_paramsCalc;

    int tickIndexByPrice(double&);
    quint16 binCount(int, int) const;
    void calcTokensSizes();

signals:
    void signalSendCalcResult(const PoolParamsCalculated&);

};



#endif // TICKOBJ_H


